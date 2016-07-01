#include <stdio.h>

#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "drivers/slave/slave.h"

STATIC int slave_read_byte(mp_hal_pin_obj_t scl, mp_hal_pin_obj_t sda, uint8_t *val, int nack) {

    uint8_t data = 0;
//test of stop
    int p = 0;
    for (int i = 7; i >= 0; i--) {
      
        while (true)  {
            if (mp_hal_pin_read(scl)==1){
//test of stop                data = (data << 1) | mp_hal_pin_read(sda);
		p = mp_hal_pin_read(sda);
		data = (data << 1) | p;
                break;
            }
        }
	while (mp_hal_pin_read(scl)==1){
		//test of stop
		if( p == 0 && mp_hal_pin_read(sda) == 1){
		    return 0;
		}
	}
        
    }
 
    mp_hal_pin_od_low(sda);
    while(true){
        if(mp_hal_pin_read(scl)==1){
            while(mp_hal_pin_read(scl)==1){
            }
            break;
        }
    }
    mp_hal_pin_od_high(sda);

    *val = data;

    return 1; // success
    
}

STATIC void slave_read(mp_hal_pin_obj_t scl, mp_hal_pin_obj_t sda, uint8_t addr, uint8_t *data, size_t len) {

    mp_uint_t irq_state = mp_hal_quiet_timing_enter();
//    mp_uint_t ticks = mp_hal_ticks_us();    
    int start=0;
    while (start==0) {
        if (mp_hal_pin_read(scl)==1  && mp_hal_pin_read(sda)==1) {
            while (mp_hal_pin_read(scl)==1){
                if (mp_hal_pin_read(sda)==0){
                    //start condition
                    start = 1;
       
                } 
//                if ((mp_uint_t)(mp_hal_ticks_us() - ticks) > 2000000) {
//                    goto timeout;
//                }
            }
        }
//        if ((mp_uint_t)(mp_hal_ticks_us() - ticks) > 2000000) {
//	        goto timeout;
//	}
    }
//check for address before here?
    while (len--) {
        if (!slave_read_byte(scl, sda, data++, len == 0)) {
            goto er;
        }
    }
    mp_hal_quiet_timing_exit(irq_state);
    return;

//timeout:
//    mp_hal_quiet_timing_exit(irq_state);
//    nlr_raise(mp_obj_new_exception_arg1(&mp_type_OSError, MP_OBJ_NEW_SMALL_INT(MP_ETIMEDOUT)));

er:
    mp_hal_quiet_timing_exit(irq_state);
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "I2C bus error"));    
    
}

STATIC mp_obj_t slave_readin(size_t n_args, const mp_obj_t *args){ 
//mp_obj_t pin_scl, mp_obj_t pin_sda, mp_obj_t addr_in, mp_obj_t nbytes_in) {
    assert(n_args==4);
    mp_obj_t pin_scl = args[0];
    mp_obj_t pin_sda = args[1];
    mp_obj_t addr_in = args[2];
    mp_obj_t nbytes_in = args[3];

    mp_hal_pin_obj_t scl = mp_hal_get_pin_obj(pin_scl);
    mp_hal_pin_obj_t sda = mp_hal_get_pin_obj(pin_sda);
    mp_hal_pin_open_drain(scl);
    mp_hal_pin_open_drain(sda);
    mp_hal_pin_od_high(scl);
    mp_hal_pin_od_high(sda);
    vstr_t vstr;
    vstr_init_len(&vstr, mp_obj_get_int(nbytes_in));
    slave_read(scl, sda, mp_obj_get_int(addr_in), (uint8_t*)vstr.buf, vstr.len);
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
MP_DEFINE_CONST_FUN_OBJ_VAR(slave_readin_obj,4, slave_readin);
