# Erico Bayani 10/29/17
# Lab 3 Decimal Converter
# this file will not work correctly unless you have arguments to the program
# be sure to add them before you try to run it.
.text
main:
	move	 $s0,$a1 
	lw	 $s0,($s0)
	la	 $a0, hello_msg # load the addr of hello_msg into $a0.
	li	 $v0, 4 # 4 is the print_string syscall.
	syscall 	# do the syscall.
	
	# beginning of my current code
	
	# produce input message
	la	 $a0, in_number
	li	 $v0, 4
	syscall
	# print program argument
	la	 $a0,($s0)
	li	 $v0, 4
	syscall		# Breakpoint here	
	
stringToInt:
	lb	 $s1, ($s0) # loads byte from address from s0 into s1
	beq	 $s1, 45, negConv # checks for negative sign in ascii; if present, stores negative 
	bne	 $s1, 45, cont # if no negative, continues with program
negConv:
	add	 $s7, $s7, $s1# stores negative sign into s7
	addi 	 $s0, $s0, 1 # goes to the next address containing the ascii argument
	j	 stringToInt
cont:
	beqz 	 $s1, exit # if the byte equals zero, go to exit
	addi 	 $s2, $s1 -48 # subtracts 48 from the ascii character to get its value, and puts the vaue into s2
	mul 	 $s3, $s3, 10 # multiplies the value in s3 by 10 to make room for another value
	add 	 $s3, $s3, $s2 # adds the value of s2 to s3
	addi 	 $s0, $s0, 1 # goes to the next address containing the ascii argument
	j	 stringToInt
	
exit:
	addi	 $s6, $s6, 0x80000000 # bitmask to get translate value into a bit
	# prints output message
	la	 $a0, out_number 
	li	 $v0, 4
	syscall
	# inverts value in s3 and adds 1 if s7 is 45
	beq	 $s7, 45, invert
	j	 cont2
invert:
	xori	 $s3, $s3, 0xffffffff
	addi	 $s3, $s3, 1
cont2: 
	# loop to output the binary until register containing bitmask equals 0
outputChar:
	sub 	 $a0, $a0, $a0 # clears a0 to make room for character output
	and	 $s4, $s3, $s6 # uses bitmask
	beqz	 $s4, cont3 # jumps to character output if s4 is zero
	# shifts value to right until LSB is 1
keepShift:
	beq	 $s4, 1, cont3 #jumps to charater output if s4 is 1
	srl	 $s4, $s4, 1 
	j 	 keepShift
cont3:
	addi	 $s5, $s4, 48 # converts value into ascii character of 1 or 0
	# prints current bit
	add	 $a0, $a0, $s5 # puuts contnents of s5 into a0 ready for printing
	li	 $v0, 11
	syscall
	srl	$s6, $s6, 1
	beqz	$s6, exit2
	j	 outputChar

exit2:
	# end of my current code
	
	# your code goes here above the exit syscall
	li	 $v0, 10 # 10 is the exit syscall.
	syscall 	# do the syscall.
# Data for the program:
.data

hello_msg: .asciiz 	"Decimal to 2SC Binary Converter\n"
in_number: .asciiz	"Input Number:   "
out_number: .asciiz 	"\nOutput Number:   "

