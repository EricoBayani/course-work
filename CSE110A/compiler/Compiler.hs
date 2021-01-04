{-# LANGUAGE TypeSynonymInstances #-}
{-# LANGUAGE FlexibleInstances    #-}

--------------------------------------------------------------------------------
-- | The entry point for the compiler: a function that takes a Text
--   representation of the source and returns a (Text) representation
--   of the assembly-program string representing the compiled version
--------------------------------------------------------------------------------

module Language.Egg.Compiler ( compiler, compile ) where

import           Data.Monoid
import           Control.Arrow                    ((>>>))
import           Prelude                  hiding (compare)
import           Control.Monad                   (void)
import           Data.Maybe
import           Data.Bits                       (shift)
import           Language.Egg.Types
import           Language.Egg.Parser     (parse)
import           Language.Egg.Checker    (check, errUnboundVar)
import           Language.Egg.Normalizer (anormal)
import           Language.Egg.Label
import           Language.Egg.Asm        (asm)


--------------------------------------------------------------------------------
compiler :: FilePath -> Text -> Text
--------------------------------------------------------------------------------
compiler f = parse f >>> check >>> anormal >>> tag >>> tails >>> compile >>> asm


--------------------------------------------------------------------------------
-- | The compilation (code generation) works with AST nodes labeled by @Ann@
--------------------------------------------------------------------------------
type Ann   = ((SourceSpan, Int), Bool)
type AExp  = AnfExpr Ann
type IExp  = ImmExpr Ann
type ABind = Bind    Ann
type ADcl  = Decl    Ann
type APgm  = Program Ann

instance Located Ann where
  sourceSpan = fst . fst

instance Located a => Located (Expr a) where
  sourceSpan = sourceSpan . getLabel

annTag :: Ann -> Int
annTag = snd . fst

annTail :: Ann -> Bool
annTail = snd


--------------------------------------------------------------------------------
compile :: APgm -> [Instruction]
--------------------------------------------------------------------------------
compile (Prog ds e) = compileBody emptyEnv e ++ concatMap compileDecl ds

compileDecl :: ADcl -> [Instruction]
compileDecl (Decl f xs e l) = [ILabel (DefStart (bindId f) 1)] 
                              ++ compileBody (paramsEnv xs) e
                              ++ [ILabel (DefEnd (bindId f) 1)] 

compileBody :: Env -> AExp -> [Instruction]
compileBody env e = funInstrs (countVars e) (compileEnv env e)

-- | @funInstrs n body@ returns the instructions of `body` wrapped
--   with code that sets up the stack (by allocating space for n local vars)
--   and restores the callees stack prior to return.

funInstrs :: Int -> [Instruction] -> [Instruction]
funInstrs n instrs
  = funEntry n
 ++ instrs
 ++ funExit
 ++ [IRet]

-- FILL: insert instructions for setting up stack for `n` local vars
funEntry :: Int -> [Instruction]
funEntry n = [  IPush (Reg EBP),
                IMov  (Reg EBP) (Reg ESP),
                ISub  (Reg ESP) (Const (4 * n))
             ]

-- FILL: clean up stack & labels for jumping to error
funExit :: [Instruction]
funExit = [   IMov (Reg ESP) (Reg EBP),
              IPop (Reg EBP)]

-- from lecture
paramsEnv :: [Bind a] -> Env
paramsEnv xs = fromListEnv (zip xids [(-2), (-3) ..])
  where
    xids     = map bindId xs
--------------------------------------------------------------------------------
-- | @countVars e@ returns the maximum stack-size needed to evaluate e,
--   which is the maximum number of let-binds in scope at any point in e.
--------------------------------------------------------------------------------
countVars :: AnfExpr a -> Int
--------------------------------------------------------------------------------
countVars (Let _ e b _)  = max (countVars e)  (1 + countVars b)
countVars (If v e1 e2 _) = maximum [countVars v, countVars e1, countVars e2]
countVars _              = 0

--------------------------------------------------------------------------------
compileEnv :: Env -> AExp -> [Instruction]
--------------------------------------------------------------------------------
compileEnv env e = compileExpr env e

compileImm :: Env -> IExp -> Instruction
compileImm env v = IMov (Reg EAX) (immArg env v)

compileBinds :: Env -> [Instruction] -> [(ABind, AExp)] -> (Env, [Instruction])
compileBinds env is []     = (env, is)
compileBinds env is (b:bs) = compileBinds env' (is <> is') bs
  where
    (env', is')            = compileBind env b

compileBind :: Env -> (ABind, AExp) -> (Env, [Instruction])
compileBind env (x, e) = (env', is)
  where
    is                 = compileEnv env e
                      <> [IMov (stackVar i) (Reg EAX)]
    (i, env')          = pushEnv x env

immArg :: Env -> IExp -> Arg
immArg _   (Number n _)  = repr n
immArg _   (Boolean b _) = repr b
immArg env e@(Id x _)    = stackVar (fromMaybe err (lookupEnv x env))
  where
    err                  = abort (errUnboundVar (sourceSpan e) x)
immArg _   e             = panic msg (sourceSpan e)
  where
    msg                  = "Unexpected non-immExpr in immArg: " <> show (void e)

param :: Env -> IExp -> Arg
param env v = Sized DWordPtr (immArg env v)

stackVar :: Int -> Arg
stackVar i = RegOffset (-4 * i) EBP



--------------------------------------------------------------------------------
-- | compileExpr stuff
--------------------------------------------------------------------------------
compileExpr :: Env -> AExp -> [Instruction]

compileExpr env v@Number {}       = [ compileImm env v  ]

compileExpr env v@Boolean {}      = [ compileImm env v  ]

compileExpr env v@Id {}           = [ compileImm env v  ]

compileExpr env e@Let {}          = is ++ compileExpr env' body
  where
    (env', is)                   = compileBinds env [] binds
    (binds, body)                = exprBinds e

compileExpr env (Prim1 o v l)     = compilePrim1 l env o v

compileExpr env (Prim2 o v1 v2 l) = compilePrim2 l env o v1 v2

compileExpr env (If v e1 e2 l)    = compileIf l env v e1 e2

compileExpr env (App f vs l) | annTail l =  compileTail env ((DefStart f 1)) vs
                             | otherwise =
                               call (DefStart f 1) [param env v | v <- vs]

-- the address is stored in EAX at the end of this compilation step
-- this version of the function only concerns the creation of the
-- data structure on the heap
compileExpr env (Tuple es l) = doAlloc env (reverse es)


compileExpr env (GetItem e1 e2 l) = compileErr l env TTuple e1 ++
                                    [IMov (Reg ECX) (Reg EAX)] ++
                                    compileErr l env TNumber e2 ++
                                    compileTupIndexErr ++
                                    [
                                     ISar (Reg EAX) (Const 1),
                                     IAdd (Reg EAX) (Const 1),
                                     IMov (Reg EBX) (Reg EAX),
                                     IMov (Reg EAX) (RegIndex ECX EBX)]
                                     
                                     
                                    

-- ==START: code for compiling and dealing with Tuples ==
-- calcAlloc calculates how many 4-word blocks to allocate
-- the result needs to be multiplied by 4 to get the actual
-- allocated block
calcAlloc :: Int -> Int
calcAlloc n | n < 4 = 1
            | otherwise = 1 + (calcAlloc (n-4))

doAlloc :: Env -> [AExp] -> [Instruction]
doAlloc env  es = [ IMov (Reg EAX) (Reg ESI),
                    IAdd (Reg EAX) (Const 1),
                    IMov (Reg ECX) (Reg EAX),
                    IMov (Reg EAX) (repr size),
                    IMov (RegOffset 0 ECX) (Reg EAX),
                    IAdd (Reg ESI) (Const ((calcAlloc (size))*16))] ++
                  populateTup env size es
                  where
                    size = length es 

populateTup :: Env -> Int -> [AExp] -> [Instruction]
populateTup _ _ [] = [IMov (Reg EAX) (Reg ECX)]
populateTup env pos (e:es) = compileExpr env e ++
                             [IMov (RegOffset (pos*4) ECX) (Reg EAX)] ++
                             populateTup env (pos - 1) es

-- compileTupNessErr :: [Instruction]
-- compileTupNessErr = [

compileTupIndexErr :: [Instruction]
compileTupIndexErr = [ICmp (Reg EAX) (RegOffset 0 ECX),
                 IJge ((DynamicErr (IndexHigh))),
                 ICmp (Reg EAX) (Const 0),
                 IJl ((DynamicErr (IndexLow)))
                 ]







-- ==END: code for compiling and dealing with Tuples == 



-- ==START: code for compiling function Apps===
--  ++GOAL++ push vs into top of given env, then use those pushed envs
--   as new parameters for the loop
-- (env', is)
--   where
--     is                 = compileEnv env e
--                       <> [IMov (stackVar i) (Reg EAX)]
--     (i, env')          = pushEnv x env
                             
compileTail :: Env -> Label -> [AExp] -> [Instruction]
compileTail env f vs = moveArgs env (reverse vs)
                       ++ funExit
                       ++ [IJmp f]
                       -- where
                       -- (env',vs') = pushParamVar env vs 

-- -- How to concat bind associated with AExp with _temp_ tag?
-- pushParamVar :: Env -> [AExp] -> (Env, [AExp])
-- pushParamVar env []     = (env , [])
-- pushParamVar env (v:vs) = (env',
--                            where
--                              env' = addEnv v

moveArgs :: Env -> [AExp] -> [Instruction]
moveArgs _ []   = []
moveArgs env (v:vs) = compileMoveArgs env v ((length vs)+1) ++ moveArgs env vs
                          --i =(reverse (take (length vs) [1,2..]))

compileMoveArgs :: Env -> AExp -> Int -> [Instruction]
compileMoveArgs env v i = [compileImm env v] ++ [IMov (paramVar i) (Reg EAX)]



paramVar :: Int -> Arg
paramVar i = RegOffset (4 * (i+1)) EBP

call :: Label -> [Arg] -> [Instruction]
call l as = [IPush a | a <- (reverse as)]
            ++
            [ICall l,
              IAdd (Reg ESP) (Const (4*(length as)))]
-- ==END:code for compiling function apps =====











-- TODO simplify compilePrim2 and compilePrim1 code to use less stuff
compilePrim1 :: Ann -> Env -> Prim1 -> IExp -> [Instruction]
compilePrim1 l env Add1 v = compileErr l env TNumber v
                            ++
                            [IAdd (Reg EAX) (repr (1::Int)),
                             IJo (DynamicErr (ArithOverflow))]

compilePrim1 l env Sub1 v = compileErr l env TNumber v
                            ++
                            [ISub (Reg EAX) (repr (1::Int)),
                             IJo (DynamicErr (ArithOverflow))]
compilePrim1 l env IsNum v = compileImm env v :
                            [
                             IAnd (Reg EAX) (typeMask TNumber),
                             ICmp (Reg EAX) (typeTag TNumber),
                             IJe  (Builtin "num_"),
                             IMov (Reg EAX) (repr False),
                             IJmp (Builtin "num_done"),
                             ILabel (Builtin "num_"),
                             IMov (Reg EAX) (repr True),
                             ILabel (Builtin "num_done")
                             ]

compilePrim1 l env IsBool v =compileImm env v :
                            [
                             IAnd (Reg EAX) (typeMask TBoolean),
                             ICmp (Reg EAX) (typeTag TBoolean),
                             IJe  (Builtin "bool_"),
                             IMov (Reg EAX) (repr False),
                             IJmp (Builtin "bool_done"),
                             ILabel (Builtin "bool_"),
                             IMov (Reg EAX) (repr True),
                             ILabel (Builtin "bool_done")
                             ]
compilePrim1 l env Print v = compileImm env v :
                            [IPush (Reg EAX),
                             ICall (Builtin "print")]


-- compileErr leaves the evaluated expression in EBX
compileErr :: Ann -> Env -> Ty -> IExp -> [Instruction] -- Helper for Errors
compileErr l env typ v = compileImm env v :
                            [IMov (Reg EBX) (Reg EAX),
                             IAnd (Reg EBX) (typeMask typ),
                             ICmp (Reg EBX) (typeTag typ),
                             IJne ((DynamicErr (TypeError typ))),
                             IMov (Reg EBX) (Reg EAX)]



compilePrim2 :: Ann -> Env -> Prim2 -> IExp -> IExp -> [Instruction]
compilePrim2 l env Plus v1 v2 = compileErr l env TNumber v1 ++
                                compileErr l env TNumber v2 ++
                                [compileImm env v1] ++ 
                                [IAdd (Reg EAX) (Reg EBX),
                                 IJo (DynamicErr (ArithOverflow))]
compilePrim2 l env Minus v1 v2 = compileErr l env TNumber v1 ++
                                compileErr l env TNumber v2 ++
                                [compileImm env v1] ++ 
                                [ISub (Reg EAX) (Reg EBX),
                                 IJo (DynamicErr (ArithOverflow))]
compilePrim2 l env Times v1 v2 = compileErr l env TNumber v1 ++
                                compileErr l env TNumber v2 ++
                                [compileImm env v1] ++ 
                                [IMul (Reg EAX) (Reg EBX),
                                 IJo (DynamicErr (ArithOverflow)),
                                 ISar (Reg EAX) (Const 1)]
                                
compilePrim2 l env Less v1 v2 = compilePrim2 l env Minus v1 v2 ++
                                [IAnd (Reg EAX) (HexConst 0x80000000),
                                 IOr  (Reg EAX) (typeMask TBoolean)]
compilePrim2 l env Greater v1 v2 = compilePrim2 l env Less v2 v1

-- Just AND the two variables and return the appropriate variable
compilePrim2 l env Equal v1 v2 =   [compileImm env v1] ++
                                   [IMov (Reg EBX) (Reg EAX)]++
                                   [compileImm env v2] ++
                                   [
                             ICmp (Reg EAX) (Reg EBX),
                             IJe  (Builtin "equals_"),
                             IMov (Reg EAX) (repr False),
                             IJmp (Builtin "equals_done"),
                             ILabel (Builtin "equals_"),
                             IMov (Reg EAX) (repr True),
                             ILabel (Builtin "equals_done")
                             ] -- yikes x2
                            -- [
                            --  ICmp (Reg EAX) (HexConst 0x00000000),
                            --  IJe  (Builtin "equals_"),
                            --  IMov (Reg EAX) (repr False),
                            --  IJmp (Builtin "equals_done"),
                            --  ILabel (Builtin "equals_"),
                            --  IMov (Reg EAX) (repr True),
                            --  ILabel (Builtin "equals_done")
                            --  ] -- yikes

compileIf :: Ann -> Env -> IExp -> AExp -> AExp -> [Instruction]
compileIf l env v e1 e2 = compileErr l env TBoolean v ++
                                   [ ICmp (Reg EAX) (repr False),
                                     IJne (BranchTrue (annTag l))] ++
                                   compileEnv env e2 ++
                                   [ IJmp (BranchDone (annTag l)),
                                     ILabel(BranchTrue (annTag l))] ++
                                   compileEnv env e1 ++
                                   [ ILabel (BranchDone (annTag l)) ]



--------------------------------------------------------------------------------
-- | Representing Values
--------------------------------------------------------------------------------

class Repr a where
  repr :: a -> Arg

instance Repr Bool where
  repr True  = HexConst 0xffffffff
  repr False = HexConst 0x7fffffff

instance Repr Int where
  repr n = Const (fromIntegral (shift n 1))


instance Repr Integer where
  repr n = Const (fromIntegral (shift n 1))

typeTag :: Ty -> Arg
typeTag TNumber   = HexConst 0x00000000
typeTag TBoolean  = HexConst 0x7fffffff
typeTag TTuple    = HexConst 0x00000001

typeMask :: Ty -> Arg
typeMask TNumber  = HexConst 0x00000001
typeMask TBoolean = HexConst 0x7fffffff
typeMask TTuple   = HexConst 0x00000007
