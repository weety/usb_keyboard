
#ifndef __ADXL345_H__
#define __ADXL345_H__

/* 旋转方向定义 */
#define ROTARY_DIR_NONE 0
#define ROTARY_DIR_F    1
#define ROTARY_DIR_B    2
#define ROTARY_DIR_ERR -1

struct rotary_data {
	INT8U rotary_curr;
	INT8U rotary_prev;
};

/*函数声明*/

extern void detect_rotary(void);


#endif

