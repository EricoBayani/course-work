# Erico Bayani 11/12/17
# Lab 4 Prime Finder
# I will be using the Sieve of Eratosthenes
.text
	# User Input
	li	$v0, 4
	la	$a0, promptPrime
	syscall
	li 	$v0, 5
	syscall
	move	$s0, $v0 # stores input into s0 and s7

	addi	$s0, $s0, -2
	add	$s4, $s4, $s0
	add	$s7, $s7, $s0
	add	$s7, $s7, 2
	# allocates memory for array by user input * bytes
	li	$v0, 9
	add	$a0, $zero, $zero
	add	$a0, $a0, $s0
	syscall
	
	# moves address of array to s1
	move 	$s1, $v0
	
	addi	$s2, $s2, 1
	
	# using t registers for array management
	add 	$t0, $t0, $s1
	
	# copies starting address to another register
	add	$s5, $s5, $s1
	
	# Creating true boolean values for each element
	# address 0x10040000 is i=2
trueSetup:	
 	beq 	$s3, $s4  exitSetup
	sb	$s2, ($t0)  
	addi	$s3, $s3, 1
	addi	$t0, $t0, 4
	j	trueSetup
exitSetup:
	# saves the ending index into another register s6

	add	$s6, $s6, $t0

	
 	nop # breakpoint here to test, t0 should be free to use
	add	$t0, $zero, $zero
	li	$v0, 4
	la	$a0, listPrime
	syscall
	add	$a0, $zero, $zero
	
	# if element is true, set multiples of element's indexes to false
iteration:
	bge	$s5, $s6 exitIteration
	lb	$t2, ($s5)

	beq 	$t2, 1 printIndex
	beqz	$t2, increment
increment:
	addi	$s5, $s5, 4
	addi	$s1, $s1, 3
	j	iteration
	# printing and iterating the indexes where element is true
printIndex:
	add	$t0, $zero, $zero
	sub	$t3, $s5, $s1
	beq	$t3, 0, skipComma
		#prints a comma
	la	$a0, comma
	li	$v0, 4
	syscall
	add	$a0, $zero, $zero
skipComma:
	# actually prints the character
	addi	$t3, $t3, 2
	li	$v0, 36
	add	$a0, $a0, $t3
	syscall
	jal	notPrime
	addi	$s5, $s5, 4
	addi	$s1, $s1, 3
	j	iteration
	
	# notPrime
	# funtion to set multiples of the square of true indexes's elements to false
notPrime:
	mul	$t4, $t3, $t3 # i^2
	bge	$t4, $s7 notPrimeEnd
	mul	$t4, $t3, 4 # translate i^2 to what needs to be added to the address
	mul	$t6, $t3, 4 # increments i with respect to addresss
	
	# for j = i2, i2+i, i2+2i, i2+3i, ..., not exceeding n:
	# A[j] := false
	# from wikipedia
primed:
	add	$t5, $t4, $s5
	bge 	$t5, $s6, notPrimeEnd
	sb 	$zero, ($t5)
	add	$t4, $t4, $t6
	j 	primed
notPrimeEnd:
	jr	$ra
exitIteration:
	# End of Program
	li 	$v0, 10
	syscall
.data
promptPrime: .asciiz "The primes from 2 to what? "
listPrime: .asciiz "\nThe prime numbers are: "
comma:	.asciiz ", "