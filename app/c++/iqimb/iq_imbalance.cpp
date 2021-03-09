
#include <math.h>
#include <iostream>
#include <vector>
#include <string>
#include <Eigen/Dense>

#include <fstream>
#include "common.hpp"

#ifdef PLATFORM
#include "zlog.h"
#endif

#define IQ_ALL_NUM_512    512
#define IQ_ALL_NUM_256    256
#define IQ_ALL_NUM_128    128
#define IQ_ALL_NUM_64      64

#ifdef PLATFORM
zlog_category_t * initLog(const char* path, char* app_name){
	int rc;
	zlog_category_t *zlog_handler = NULL;

	rc = zlog_init(path);

	if (rc) {
		printf("init serverLog failed\n");
		return NULL;
	}

	zlog_handler = zlog_get_category(app_name);

	if (!zlog_handler) {
		printf("get cat fail\n");

		zlog_fini();
		return NULL;
	}

	return zlog_handler;
}

void closeLog(){
	zlog_fini();
}

#endif

char *get_prog_name(char *argv)
{
	int len = strlen(argv);
	int i;
	char *tmp = argv;
	
	for(i=len; i >=0; i--)
	{
		if(tmp[i] == '/'){
			i++;
			break;
		}
	}
	
	if(-1 == i){
		i = 0;
	}

	return argv + i;
}

// iq imbalance process
int cal_iq_imbalance(float *h_alpha_i, float *h_alpha_q, 
                     float *h_beta_i, float *h_beta_q, int length,
					 float *c1, float *c2){
	// init cv_h_alpha
	Eigen::VectorXf v_h_alpha_i;
	Eigen::VectorXf v_h_alpha_q;
	v_h_alpha_i.resize(length,1);
	v_h_alpha_q.resize(length,1);
	v_h_alpha_i.setZero();
	v_h_alpha_q.setZero();

	// init cv_h_beta
	Eigen::VectorXf v_h_beta_i;
	Eigen::VectorXf v_h_beta_q;
	v_h_beta_i.resize(length,1);
	v_h_beta_q.resize(length,1);
	v_h_beta_i.setZero();
	v_h_beta_q.setZero();

	for(int i=0;i<length;i++){
		v_h_alpha_i(i) = h_alpha_i[i];
		v_h_alpha_q(i) = h_alpha_q[i];
		
		v_h_beta_i(i) = h_beta_i[i];
		v_h_beta_q(i) = h_beta_q[i];
	}

	Eigen::VectorXcf v_h_alpha;
	v_h_alpha.resize(length,1); 
	v_h_alpha.setZero();
    v_h_alpha.real() << v_h_alpha_i; 
    v_h_alpha.imag() << v_h_alpha_q;

	Eigen::VectorXcf v_h_beta;
	v_h_beta.resize(length,1); 
	v_h_beta.setZero();
    v_h_beta.real() << v_h_beta_i; 
    v_h_beta.imag() << v_h_beta_q;

	// std::cout << "cv_h_alpha = " << std::endl << v_h_alpha << std::endl;
	// std::cout << "cv_h_beta = " << std::endl << v_h_beta << std::endl;

	// process complex matrix
	Eigen::VectorXcf v_a_plus_conjb = v_h_alpha + v_h_beta.conjugate();

	Eigen::MatrixXcf H_tmp;
	H_tmp.resize(length,3);
	H_tmp.setZero();

	H_tmp.col(0) << v_a_plus_conjb;
	H_tmp.col(1).tail(63) << v_a_plus_conjb.head(63);
	H_tmp.col(2).tail(62) << v_a_plus_conjb.head(62);

	Eigen::MatrixXcf HT_H = (H_tmp.adjoint() * H_tmp);

	Eigen::MatrixXcf result;

	float x1 = HT_H.determinant().real();
	float x2 = HT_H.determinant().imag();

	if(x1 * x1 + x2 * x2 > 0.000000001){
		result = HT_H.inverse() * H_tmp.adjoint() * (v_h_alpha - v_h_beta.conjugate());
	}else{
		std::cout << "HT_H = " << HT_H.determinant() << std::endl << HT_H << std::endl;

		std::cout << "result = " << std::endl << result << std::endl;

		return -1;
	}

	std::cout << "result = " << std::endl << result << std::endl;

	c1[0] = result.real()(0);
	c1[1] = result.real()(1);
	c1[2] = result.real()(2);

	c2[0] = result.imag()(0);
	c2[1] = result.imag()(1);
	c2[2] = result.imag()(2);

	return 0;

}

// length = 128 , up factor = 2
int findMaxPath(float *IData, float *QData, int length){
	std::cout << " findMaxPath " << std::endl;
	float sum_1 = 0;
	float sum_2 = 0;
	int max_idx = 0;
	for(int i=0;i<length;i=i+2){
		sum_1 = sum_1 + IData[i] * IData[i] + QData[i] * QData[i];
	}

	for(int i=1;i<length;i=i+2){
		sum_2 = sum_2 + IData[i] * IData[i] + QData[i] * QData[i];
	}

	if(sum_1 < sum_2){
		max_idx = 1;
	}
	return max_idx;
}

// 128 -> 64
int downsample(float *IData, float *QData, int max){
	int num = IQ_ALL_NUM_128;
	int cnt = 0;

	float tmp_I[IQ_ALL_NUM_64] = {0};
	float tmp_Q[IQ_ALL_NUM_64] = {0};

	for(int i = max; i<num;i = i+2){
		tmp_I[cnt] = IData[i];
		tmp_Q[cnt] = QData[i];
		cnt = cnt + 1;
		if(cnt == IQ_ALL_NUM_64){
			break;
		}
	}

	for(int i=0;i<cnt;i++){
		IData[i] = tmp_I[i];
		QData[i] = tmp_Q[i];
	}

	return cnt;
}

// file operation
int read_from_file(float *raw_IData, float *raw_QData, char *file_path){
	std::ifstream file(file_path);
	float *raw_data = new float[IQ_ALL_NUM_512];
	int cnt = 0;
	while( ! file.eof() ){
		file >> raw_data[cnt++];
		if(cnt == IQ_ALL_NUM_512){
			break;
		}
	}
	file.close();

	int num = 0;
	for(int i=0;i<cnt;i++){
		if(i%2 == 0){
			raw_IData[num] = raw_data[i];
		}else{
			raw_QData[num] = raw_data[i];
			std::cout << raw_IData[num] << " " << raw_QData[num] << std::endl;
			num++;
		}
	}

	std::cout << std::endl;

	return 0;
}

int32_t tranform_float2int(float f_value){
	int32_t value = 0;
	float tmp_value = 0;
	tmp_value = f_value * 16384;

	value = (int32_t)tmp_value;
	return value;
}

#ifdef PLATFORM
void compensate_by_devmem(int addr, int32_t value, zlog_category_t *zlog_handler){
	char command[128];
	sprintf(command, "devmem 0x%x 32 0x%x", addr, value);
	system(command);
	zlog_info(zlog_handler, "devmem 0x%x 32 0x%x \n", addr, value);
}

void init_fpga(zlog_category_t *zlog_handler){
	int32_t value_all = 0x0;
	int addr[3] = {0x43c20150, 0x43c20154, 0x43c20158};
	for(int i=0;i<3;i++){
		value_all = 0x0;
		if(i==0){
			value_all = 0x40000000;
		}
		compensate_by_devmem(addr[i],value_all, zlog_handler);
	}
}

void write_to_fpga(float *c1, float *c2, zlog_category_t *zlog_handler){
	if(c1[0] < 0.7 || c1[0] > 1.3){
		zlog_error(zlog_handler, "c1[0] = %f ---- ", c1[0]);
		init_fpga(zlog_handler);
		return;
	}

	int32_t value_all = 0;
	int32_t value_1 = 0;
	int32_t value_2 = 0;
	int addr[3] = {0x43c20150, 0x43c20154, 0x43c20158};
	for(int i=0;i<3;i++){	
		value_1 = tranform_float2int(c1[i]);
		value_2 = tranform_float2int(c2[i]);
		value_all = (value_1 << 16) + value_2;
		zlog_info(zlog_handler, "c1[%d] = %f - %x, c2[%d] = %f - %x", i, c1[i], value_1, i, c2[i], value_2);
		compensate_by_devmem(addr[i],value_all, zlog_handler);
	}
}
#endif

int main(int argc, char* argv[])
{
	if(argc != 3){
		printf(" /run/media/mmcblk1p2/iqswap/raw_data.txt \n");
		return 0;
	}

	char *app_name = "iq_imbalance";

#ifdef PLATFORM	
	zlog_category_t *zlog_handler = initLog(argv[2],app_name);
	// zlog_category_t *zlog_handler = initLog(argv[2],get_prog_name(argv[0]));

	zlog_info(zlog_handler,"******************** start iq_imbalance process ********************************\n");

	zlog_info(zlog_handler,"this version built time is:[%s  %s]\n",__DATE__,__TIME__);
#endif
	// get data from fpga..... 
	// fpag data sequence transfer to arm sequence....
	// 256 factor = 2; (0:127) = alpha , (128:255) = beta 
	float *raw_IData =  new float[IQ_ALL_NUM_256];
	float *raw_QData = new float[IQ_ALL_NUM_256];

	read_from_file(raw_IData, raw_QData, argv[1]);

	// start process in arm 
	// 128 factor = 2
	float *raw_alpha_IData = new float[IQ_ALL_NUM_128];
	float *raw_alpha_QData = new float[IQ_ALL_NUM_128];
	float *raw_beta_IData  = new float[IQ_ALL_NUM_128];
	float *raw_beta_QData  = new float[IQ_ALL_NUM_128];

	// fpga data to alpha and beta part...
	for(int i=0;i<IQ_ALL_NUM_128;i++){
		raw_alpha_IData[i] = raw_IData[i];
		raw_beta_IData[i] = raw_IData[IQ_ALL_NUM_128+i];
		raw_alpha_QData[i] = raw_QData[i];
		raw_beta_QData[i] = raw_QData[IQ_ALL_NUM_128+i];
	}

	int max_alpha = findMaxPath(raw_alpha_IData, raw_alpha_QData, IQ_ALL_NUM_128);
	int num_alpha = downsample(raw_alpha_IData, raw_alpha_QData, max_alpha);
	int num_beta = downsample(raw_beta_IData, raw_beta_QData, max_alpha);

#ifdef PLATFORM	
	zlog_info(zlog_handler, "max_alpha = %d" ,max_alpha);
	zlog_info(zlog_handler, "num_alpha = %d , num_beta = %d \n" , num_alpha, num_beta);
#endif
	/* 	
		num = 64 , 
		raw_alpha_IData = h_alpha_i
		raw_alpha_QData = h_alpha_q
		raw_beta_IData = h_beta_i
		raw_beta_QData = h_beta_q
	*/

	float c1[3] = {0};
	float c2[3] = {0};

	if(0==cal_iq_imbalance(raw_alpha_IData,raw_alpha_QData,raw_beta_IData,raw_beta_QData,IQ_ALL_NUM_64, c1, c2)){
#ifdef PLATFORM	
		write_to_fpga(c1,c2,zlog_handler);
#endif
	}else{
#ifdef PLATFORM	
		init_fpga(zlog_handler);
		zlog_info(zlog_handler, "not set iq_imbalance compensate !");
#endif
		return 0;
	}

#ifdef PLATFORM	
	zlog_info(zlog_handler, "------------ end iq_imbalance !");
#endif

	return 0;
}

