.global arch_copy_user 

#same function works for both copy to and copy from user

arch_copy_user:
    #a0 > t0 : src (current loc)
    #a1 : size (left to go)
    #a2 : dest (current dest)
    #t2 : temp moving
    #a0 becomes response, 0 success, any more failed
    mv t0, a0                       #cur_loc = user_addr
    li a0, 0x0                      #response = 0
    j copy_byte

copy_byte:
    bgt a0, zero, return_response   #if response > 0 then return
    beq a1, zero, return_response   #if size == 0 then return
    lb t2, 0(t0)                    #data = *cur_loc
    sb t2, 0(a2)                    #*dest = data

    addi t0, t0, 1                  #cur_loc = cur_loc + 1
    addi a2, a2, 1                  #*dest = *dest + 1
    addi a1, a1, -1                 #size = size - 1
    j copy_byte

return_response:
    ret