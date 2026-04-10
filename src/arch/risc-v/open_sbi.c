long open_sbi_set_timer(long clock_to_run) {
    register long a0 asm("a0") = clock_to_run;
    register long a6 asm("a6") = 0;
    register long a7 asm("a7") = 0x54494D45;
    asm("ecall"
        : "+r"(a0) //outputs
        : "r"(a6), "r"(a7) //inputs
        : "memory" //says ram could have had updated values
    );
    return a0;
}





//later to add
//sbi_set_timer 0 0x00 0x54494D45
//sbi_send_ipi 0 0x04 0x735049
//sbi_shutdown 0 0x08 0x53525354

//sbi_remote_fence_i 0 0x52464E43
//sbi_remote_sfence_vma 1 0x52464E43
//sbi_remote_sfence_vma_asid 2 0x52464E43
//sbi_remote_hfence_gvma_vmid 3 0x52464E43
//sbi_remote_hfence_gvma 4 0x52464E43
//sbi_remote_hfence_vvma_asid 5 0x52464E43
//sbi_remote_hfence_vvma 6 0x52464E43

//sbi_hart_start 0 0x48534D
//sbi_hart_stop 1 0x48534D
//sbi_hart_get_status 2 0x48534D
//sbi_hart_suspend 3 0x48534D

//sbi_system_reset 0 0x53525354