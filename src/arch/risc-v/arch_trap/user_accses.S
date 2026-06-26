.global arch_read_user 

arch_read_user:
    #a0 > t0 : src (current loc)
    #a1 : size (left to go)
    #a2 : dest (current dest)
    #t2 : temp moving
    #a0 becomes response, 0 success, any more failed
    mv t0, a0
    li a0, 0x0
    j copy_byte

copy_byte:
    bgt a0, zero, return_response #for performance just return now if error occurred.
    beq a1, zero, return_response #return when no bytes left
    lb t2, 0(t0)
    sb t2, 0(a2)

    addi t0, t0, 1
    addi a2, a2, 1
    addi a1, a1, -1
    j copy_byte

return_response:
    ret