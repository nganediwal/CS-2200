! Spring 2022 Revisions by Andrej Vrtanoski

! This program executes pow as a test program using the LC 22 calling convention
! Check your registers ($v0) and memory to see if it is consistent with this program

! vector table
vector0:
        .fill 0x00000000                        ! device ID 0
        .fill 0x00000000                        ! device ID 1
        .fill 0x00000000                        ! ...
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000                        ! device ID 7
        ! end vector table

main:	lea $sp, initsp                         ! initialize the stack pointer
        lw $sp, 0($sp)                          ! finish initialization

        lea $t0, vector0                        ! DONE: Install timer interrupt handler into vector table
        lea $t1, timer_handler
        sw  $t1, 0($t0)

        lea $t1, toaster_handler                ! DONE: Install toaster interrupt handler into vector table
        sw  $t1, 1($t0)

        lea $t0, minval
        lw $t0, 0($t0)
        addi $t1, $zero, 65534                  ! store 0000ffff into minval (to make comparisons easier)
        sw $t1, 0($t0)

        ei                                      ! Enable interrupts

        lea $a0, BASE                           ! load base for pow
        lw $a0, 0($a0)
        lea $a1, EXP                            ! load power for pow
        lw $a1, 0($a1)
        lea $at, POW                            ! load address of pow
        jalr $ra, $at                           ! run pow
        lea $a0, ANS                            ! load base for pow
        sw $v0, 0($a0)

        halt                                    ! stop the program here
        addi $v0, $zero, -1                     ! load a bad value on failure to halt

BASE:   .fill 2
EXP:    .fill 8
ANS:	.fill 0                                 ! should come out to 256 (BASE^EXP)

POW:    addi $sp, $sp, -1                       ! allocate space for old frame pointer
        sw $fp, 0($sp)

        addi $fp, $sp, 0                        ! set new frame pointer

        bgt $a1, $zero, BASECHK                 ! check if $a1 is zero
        br RET1                                 ! if the exponent is 0, return 1

BASECHK:bgt $a0, $zero, WORK                    ! if the base is 0, return 0
        br RET0

WORK:   addi $a1, $a1, -1                        ! decrement the power
        lea $at, POW                            ! load the address of POW
        addi $sp, $sp, -2                       ! push 2 slots onto the stack
        sw $ra, -1($fp)                         ! save RA to stack
        sw $a0, -2($fp)                         ! save arg 0 to stack
        jalr $ra, $at                           ! recursively call POW
        add $a1, $v0, $zero                     ! store return value in arg 1
        lw $a0, -2($fp)                         ! load the base into arg 0
        lea $at, MULT                           ! load the address of MULT
        jalr $ra, $at                           ! multiply arg 0 (base) and arg 1 (running product)
        lw $ra, -1($fp)                         ! load RA from the stack
        addi $sp, $sp, 2

        br FIN                                  ! unconditional branch to FIN

RET1:   add $v0, $zero, $zero                   ! return a value of 0
	addi $v0, $v0, 1                        ! increment and return 1
        br FIN                                  ! unconditional branch to FIN

RET0:   add $v0, $zero, $zero                   ! return a value of 0

FIN:	lw $fp, 0($fp)                          ! restore old frame pointer
        addi $sp, $sp, 1                        ! pop off the stack
        jalr $zero, $ra

MULT:   add $v0, $zero, $zero                   ! return value = 0
        addi $t0, $zero, 0                      ! sentinel = 0
AGAIN:  add $v0, $v0, $a0                       ! return value += argument0
        addi $t0, $t0, 1                        ! increment sentinel
        blt $t0, $a1, AGAIN                     ! while sentinel < argument, loop again
        jalr $zero, $ra                         ! return from mult

timer_handler:
                                              ! TODO FIX ME
        addi $sp, $sp, -1                     ! FIX ME
        sw $k0, 0($sp)                        ! save return address ($k0) on stackframe
        ei                                    ! enable interrupts

        addi $sp, $sp, -2
        sw $t0, 0($sp)
        sw $t1, 1($sp)
     
     
        lea $t0, ticks                        ! mem address of ticks
        lw $t0, 0($t0)                        !t0 = MEM[TICKS] = 0xFFFF
        lw $t1, 0($t0)                        !t1 = MEM[0xffff] = counter
        addi $t1, $t1, 1                      !counter++
        sw $t1, 0($t0)                        !MEM[0xffff] = counter + 1

       
        lw $t0, 0($sp)
        lw $t1, 1($sp)
        addi $sp, $sp, 2

        di                                     ! disable interrupts
        lw $k0, 0($sp)                         ! 0x0, device id
        addi $sp, $sp, 1
        reti

toaster_handler:
!         ! retrieve the data from the device and check if it is a minimum or maximum value
!         ! then calculate the difference between minimum and maximum value
!         ! (hint: think about what ALU operations you could use to implement subract using 2s compliment)

          add $zero, $zero, $zero                 ! TODO FIX ME 
                                                  ! FIX ME

          addi  $sp, $sp, -1
          sw $k0, 0($sp)                          ! save return address ($k0) on stackframe
          ei
          addi $sp, $sp, -3                                      ! enable interrupts
          sw $t0, 0($sp)
          sw $t1, 1($sp)
          sw $t2, 2($sp)

          in $t0, 1                               ! store incoming data into toaster device id
          lea $t1, maxval                         ! mem address of maxVal
          lw  $t1, 0($t1)                         ! this holds MEM[maxval] = 0xFFFD
          lw  $t2, 0($t1)                         ! this holds MEM[0xFFFD] = maxVal
          bgt $t0, $t2, new_max                   ! if in val is greater than curr maxVal, branch to newMax to update it

cont:
         lea $t1, minval                         ! mem address of minVal
         lw  $t1, 0($t1)
         lw  $t2, 0($t1)
         blt $t0, $t2, new_min                   ! if in val is less than curr minVal, branch to newMin to update it
         br difference

new_max: 
         sw  $t0, 0($t1)                         ! store new maxval from int into memory address of maxval
         br difference

new_min: 
         sw  $t0, 0($t1)                         ! store new minval from int into memory address of minval

difference:
         lea $t0, minval
         lw  $t0, 0($t0)
         lw  $t1, 0($t0)                         ! now we have minVal
         nand $t1, $t1, $t1
         addi $t1, $t1, 1                        ! 2's complement, t1 = -minVal 
        
         lea $t0, maxval
         lw  $t0, 0($t0)
         lw  $t2, 0($t0)                         ! maxVal in t2, t2 = -minval, add both
         add $t2, $t2, $t1                       ! maxVal + (-minVal) = difference = t2

         lea $t0, range
         lw $t0, 0($t0)
         sw $t2, 0($t0)                          ! store value of max - min into mem address 0xFFFE


         lw $t0, 0($sp)
         lw $t1, 1($sp)
         lw $t2, 2($sp)
         addi $sp, $sp, 3
         di
         lw $k0, 0($sp) 
         addi $sp, $sp, 1
         reti



initsp: .fill 0xA000
ticks:  .fill 0xFFFF
range:  .fill 0xFFFE
maxval: .fill 0xFFFD
minval: .fill 0xFFFC