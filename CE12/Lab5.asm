# Erico Bayani 11/26/17
# Lab 5 Vigenère cipher
.data
key_text: .asciiz "The given key is: "
clear_text: .asciiz "\nThe given text is: "
encrypted_text: .asciiz "\nThe encrypted text is: "
decrypted_text: .asciiz "\nThe decrypted text is: "
encrypted_string: .space 101
decrypted_string: .space 101
.text
# main
main:
	# print key message and key
	move	 $s0, $a1
	li	 $v0, 4
	la	 $a0, key_text
	syscall
	
	lw	 $a0, ($s0)
	syscall
	#
	# print clear text message and clear text
	la	 $a0, clear_text
	syscall
	
	lw	 $a0, 4($s0)
	syscall
	#
	
	jal	encrypt
	# code that prints encrypted message goes here
	la	$a0, encrypted_text
	syscall
	la	$a0, encrypted_string
	syscall
	#
	
	jal	decrypt
	
	# code that prints decrpyted message goes here
	la	$a0, decrypted_text
	syscall
	la	$a0, decrypted_string
	syscall	
	#
	
	# end proram
	li	 $v0, 10
	syscall
	
# start of funtions
	# Encrypt Function
encrypt:
	# saves key address and cleartext address into s1 and s2 respectively for later use
	lw 	$s1, ($s0)
	lw	$s2, 4($s0)
	# finds the lengths of the key and the cleartext and stores those values in t1 and t2 respectively
	findEncryptKeyLength:
	lb	$t0, ($s1)
	beqz	$t0, endEncryptKeyLength
	addi	$t1, $t1, 1
	addi	$s1, $s1, 1
	j	findEncryptKeyLength
	endEncryptKeyLength:


	findClearLength:
	lb	$t0, ($s2)
	beqz	$t0, endClearLength
	addi	$t2, $t2, 1
	addi	$s2, $s2, 1
	j	findClearLength
	endClearLength:
	# i = t7
	lw	$s2, 4($s0)	
	la	$t6, encrypted_string # load address of encrypted string for iteration
	la	$s3, encrypted_string # load address of encrypted string for later use
	# loop that encrypts clear text according to cipher.c code
	encryptClear:
	lw 	$s1, ($s0) # address of key
	lb	$t3, ($s2) # load byte from cleartext
	beq 	$t7, $t2, endEncryptClear # ends loop when we match the length of the clear text
	rem	$t5, $t7, $t1 # i % lk
	add	$s1, $s1, $t5 # key[i % lk]
	lb 	$t4, ($s1) # load byte from key 
	add	$t5, $t3, $t4 # (cleartext[i] + key[i % lk])
	rem	$t5, $t5, 128 # (cleartext[i] + key[i % lk]) % ASCII;
	sb 	$t5, ($t6) # stores value into address of allotted array encrypted_string
	addi	$t7, $t7, 1 # i += 1
	addi	$s2, $s2, 1 # i += 1
	addi	$t6, $t6, 1 # increments encrypted_string by 1 for storage 
	j	encryptClear
	endEncryptClear:
	# adds a null character to encrypted string
	addi	$t6, $t6, 1 
	sb 	$zero, ($t6)
	# clears t registers for later use
	add	$t0, $zero, $zero
	add	$t1, $zero, $zero	
	add	$t2, $zero, $zero	
	add	$t3, $zero, $zero	
	add	$t4, $zero, $zero	
	add	$t5, $zero, $zero	
	add	$t6, $zero, $zero
	add	$t7, $zero, $zero	
			
	jr	$ra
	# Decrypt Function
decrypt:
	# saves key address into s4 and s5 later use, encrypted_string already is in s3
	lw 	$s4, ($s0)
 
	# finds the lengths of the key and the cleartext and stores those values in t1 and t2 respectively
	findDecryptKeyLength:
	lb	$t0, ($s4)
	beqz	$t0, endDecryptKeyLength
	addi	$t1, $t1, 1
	addi	$s4, $s4, 1
	j	findDecryptKeyLength
	endDecryptKeyLength:
	
	findCipherLength:
	lb	$t0, ($s3)
	beqz	$t0, endCipherLength
	addi	$t2, $t2, 1
	addi	$s3, $s3, 1
	j	findCipherLength
	endCipherLength:
	# i = t7
	la	$t6, decrypted_string # load address of encrypted string for iteration
	la	$s3, encrypted_string # load address of encrypted string for later use
	# loop that decrypts ciphertext according to cipher.c code
	decryptClear:
	lw 	$s4, ($s0) # address of key
	lb	$t3, ($s3) # load byte from encrypted_string
	beq	$t7, $t2, endDecryptClear # ends loop when we match the length of the encrypted string
	rem	$t5, $t7, $t1 # i % lk
	add	$s4, $s4, $t5 # key[i % lk]
	lb 	$t4, ($s4) # load byte from key 
	sub	$t5, $t3, $t4 # (ciphertext[i] - key[i % lk])
	rem	$t5, $t5, 128 # (ciphertext[i] - key[i % lk]) % ASCII;
	bge	$t5, 0xffffffff, notNegative # handles negative values from subtraction
	xori	$t5, $t5, 0xffffff80
	notNegative:
	sb 	$t5, ($t6) # stores value into address of allotted array decrypted_string
	addi	$t7, $t7, 1 # i += 1
	addi	$s3, $s3, 1 # i += 1
	addi	$t6, $t6, 1 # increments decrypted_string by 1 for storage 
	j	decryptClear
	endDecryptClear:	
	# adds a null character to decrypted string
	addi	$t6, $t6, 1 
	sb 	$zero, ($t6)
	# clears t registers for later use
	add	$t0, $zero, $zero
	add	$t1, $zero, $zero	
	add	$t2, $zero, $zero	
	add	$t3, $zero, $zero	
	add	$t4, $zero, $zero	
	add	$t5, $zero, $zero	
	add	$t6, $zero, $zero
	add	$t7, $zero, $zero
	jr	$ra
# end of functions

