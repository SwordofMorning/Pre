/**
 * @FilePath     : /project/pre/src/media/algo/ir_auto_focusing.cpp
 * @Description  :  
 * @Author       : xiaojing
 * @Version      : 0.0.1
 * @LastEditors  : xiaojing
 * @LastEditTime : 2024-12-27 14:00:12
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
/**
 * @FilePath     : /project/pre/src/media/algo/ir_auto_focusing.cpp
 * @Description  :  
 * @Author       : xiaojing
 * @Version      : 0.0.1
 * @LastEditors  : xiaojing
 * @LastEditTime : 2024-12-27 08:43:04
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
/**
 * @FilePath     : /project/pre/src/media/algo/ir_auto_focusing.cpp
 * @Description  :  
 * @Author       : xiaojing
 * @Version      : 0.0.1
 * @LastEditors  : xiaojing
 * @LastEditTime : 2024-12-26 15:29:35
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#include "ir_auto_focusing.h"
#include<string>
#include <iostream>

#include <filesystem>
#include <sstream>

#include "../../config/config.h"


int IR_HIGH = 512;
int IR_WIDTH = 640;
int IR_W_Cut = 150;
int IR_H_Cut = 150;
int IR_Motor_Step_Total = 2000;
int IR_MOTOR_MUC_RETURN_FREQ = 20;
bool Print_Debug_Algo_Log_Info = true;


// void IR_Show_16(irData16)
// {
//     uint16_t* yuyv16 = (uint16_t*)v4l2_ir_dvp_buffer_global[v4l2_ir_dvp_buffer_global_index].start;

//     uint16_t tmpVal =0;
//     int index, i, j;
//     int count0 = 0;

//     for (i = 0; i < IR_HIGH; i++){
//         for(j = 0; j < IR_WIDTH; j++)
//         {
//             index = (i * IR_WIDTH + j);
//             irData16.at<uint16_t>(i, j) = yuyv16[index];
//         }
//     }

// }


// void ir_save_16_png(){
//     std::string ROOT_DIR = "/root/xj/img16/"


//     auto now = std::chrono::system_clock::now();
//     auto now_c = std::chrono::system_clock::to_time_t(now);

//     // 将时间转换为字符串
//     std::stringstream ss;
//     ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d_%H-%M-%S");
//     std::string folderName = ss.str();


//      // 创建文件夹路径
//     std::filesystem::path dirPath = folderName;

//     // 检查文件夹是否已经存在
//     if (!std::filesystem::exists(dirPath)) {
//         // 创建文件夹
//         if (std::filesystem::create_directory(dirPath)) {
//             std::cout << "Directory created successfully: " << dirPath << std::endl;
//         } else {
//             std::cerr << "Error creating directory: " << dirPath << std::endl;
//         }
//     } else {
//         std::cout << "Directory already exists: " << dirPath << std::endl;
//     }



//     cv::Mat irRaw(IR_HIGH, IR_WIDTH, CV_16U);


//     for(int i =0; i<6000; i++){


//         std::string stri = std::to_string(i);
//         std::string filename= ROOT_DIR + "img/" + stri + ".jpg";
//         //cv::imwrite(str1, irData);

        
//         filename= ROOT_DIR + "img16/" + stri + ".png";
//         while(v4l2_ir_dvp_buffer_global_index == last_index){
//             usleep(1000);
//         }
//         last_index = v4l2_ir_dvp_buffer_global_index;

//         cv::Mat irData16(IR_HIGH, IR_WIDTH, CV_16U);


//         IR_Show_16(irData16);

//         cv::imwrite(filename, irData16);

//         std::cout<< filename << std::endl;


       
        
//     }
 


    
// }


void IR_Show_O(cv::Mat &irData){
    cv::Mat irRaw(IR_HIGH, IR_WIDTH, CV_16U);
    uint16_t* yuyv16 = (uint16_t*)v4l2_ir_dvp_buffer_global[v4l2_ir_dvp_buffer_global_index].start;

    uint16_t tmpVal =0;
    int index, i, j;
    int count0 = 0;

    for (i = 0; i < IR_HIGH; i++){
        for(j = 0; j < IR_WIDTH; j++)
        {
            index = (i * IR_WIDTH + j);
            irRaw.at<uint16_t>(i, j) = yuyv16[index];
        }
    }
    cv::medianBlur(irRaw, irRaw, 3);
    cv::normalize(irRaw, irData, 0, 255, cv::NORM_MINMAX, CV_8U);

}




// 截取图为了计算5x5均值滤波


// 清晰度评价函数
void eig_my(uint8_t* pixels, float* eig) {

    int index = 0;
    double out = 0.0;
    for (int i = 0; i < IR_W_Cut - 1; i++) {
        for (int j = 0; j < IR_H_Cut - 1; j++) {
            index = i + j * IR_W_Cut;
            printf("pixels[index] = %f\n", pixels[index] );
            out += (pixels[index] - pixels[index + 1]) * (pixels[index] - pixels[index + 1]) + (pixels[index] - pixels[i + (j + 1) * IR_W_Cut]) * (pixels[index] - pixels[i + (j + 1) * IR_W_Cut]);

        }
    }
    out = out / IR_W_Cut / IR_H_Cut;
    printf("eig out = %f\n", out);
    *eig = out;

}


// 函数定义：用于查找数组中的最大值
int findMaxIndex(float* array, size_t size) {
    if (size == 0) {
        // 如果数组为空，返回一个默认值或处理错误
        return -1; // 这里假设 -1 表示无效值
    }

    // 假设数组的第一个元素是最大值
    float max = array[0];
    int index = 0;

    // 遍历数组，找到最大值
    for (size_t i = 0; i < size; i++) {
        if (array[i] > max) {
            max = array[i];
            index = i;
        }
       // printf("eig value [%d] = %f\n", i, array[i]);
    }

    return index;
}


float findMaxValue(float* array, size_t size) {
    if (size == 0) {
        // 如果数组为空，返回一个默认值或处理错误
        return -1; // 这里假设 -1 表示无效值
    }

    // 假设数组的第一个元素是最大值
    float max = array[0];
    int index = 0;

    // 遍历数组，找到最大值
    for (size_t i = 0; i < size; i++) {
        if (array[i] > max) {
            max = array[i];
            index = i;
        }
       // printf("eig value [%d] = %f\n", i, array[i]);
    }

    return max;
}


 

double IR_Get_EIG(cv::Mat &irGray255){

    //cv::Mat grayImg = irGray255(cv::Rect(centerX, centerY, 150, 150));
    cv::Mat kern1 = (cv::Mat_<char>(2, 1) << -1, 1);
    cv::Mat kern2 = (cv::Mat_<char>(1, 2) << -1, 1);
    cv::Mat engImg1, engImg2;
    cv::filter2D(irGray255, engImg1, 5, kern1);
    cv::filter2D(irGray255, engImg2, 5, kern2);
    cv::Mat resImg = engImg1.mul(engImg1) + engImg2.mul(engImg2);
    double OutValue = cv::mean(resImg)[0];
    return OutValue;
}


double IR_Get_EIG_test(cv::Mat &irGray255){

    double result = .0f;

  	for (int i = 0; i < irGray255.rows-1; ++i){
		for (int j = 0; j < irGray255.cols - 1; ++j){
            //printf("irGray255.at<u_char>(%d,%d) = %d\n", i,j, irGray255.at<u_char>(i,j));
			result += pow(irGray255.at<u_char>(i,j+1)-irGray255.at<u_char>(i,j), 2) + pow(irGray255.at<u_char>(i+1,j)-irGray255.at<u_char>(i,j), 2);
		}
	}
    double outValue = result/irGray255.total();
    return outValue;
}



int eig_index_glob = 0;

float get_eig(int centX, int centY) {


    //printf("centX = %d\n", centX);
    // 从摄像头捕获一帧图像
    cv::Mat grayImage(IR_HIGH, IR_WIDTH, CV_8U);
   // IR_show_for_auto_focusing(grayImage, centX, centY);
   // usleep(100000);
    IR_Show_O(grayImage);

    // eig_index_glob +=1;
    // std::string ROOT_DIR = "/root/xj/";
    // std::string stri = std::to_string(eig_index_glob);
    // std::string filename= ROOT_DIR + "img/"+stri + ".jpg";
    // cv::imwrite(filename, grayImage);
   // printf("eig_index_glob = %d\n", eig_index_glob);



    cv::blur(grayImage, grayImage, cv::Size(3, 3));



    int xLeft = centX - IR_W_Cut / 2;
    int yLeft = centY - IR_H_Cut / 2;
   // printf("xLeft = %d; yLeft = %d\n", xLeft, yLeft);
    if(xLeft<IR_W_Cut) xLeft = IR_WIDTH - IR_W_Cut + 1;
    if(yLeft<IR_H_Cut) yLeft = IR_HIGH - IR_H_Cut + 1;

    cv::Rect rect(xLeft, yLeft, IR_W_Cut, IR_H_Cut); //创建一个Rect对象

   // cv::Mat grayImage;



    cv::Mat imgcut = grayImage(rect);//.clone();


   // cv::normalize(imgcut, imgcut, 0, 255, cv::NORM_MINMAX, CV_8U);


    //cv::blur(imgcut, imgcut, cv::Size(3, 3));



    //cv::imwrite("/root/xj/img/imgcut.jpg", imgcut);



    float eig_value = 0.0;



    eig_value = IR_Get_EIG(imgcut);
    if(Print_Debug_Algo_Log_Info) printf( "eig_my = %f \n", eig_value);

    // 显示捕获的帧


    return eig_value;
}




class Auto_Focusing{
    public:
    Motor& IR_Motor;
    int x;
    int y;
    float eig;
    float T_diff = 2;  //电机变方向的依据
    float T_cut = 1;
    int diff_down_times = 0;
    int diff_up_times = 0;

    float last_eig = 0.0;
    float max_eig = 0.0;
    int max_motor_pos = 0;
    int current_motor_pos = 0;
    bool is_immediate_extremum = false;
    int motor_pos_list[600] = { 0 };
    float eig_values_list[600] = { 0 };
    int list_index = 0;
    int32_t motor_pulse = 30;
    int T_down = 0;
    int T_up = 0;
    int pos = 0;
    int motor_return_pos_list[600] = { 0 }; 

    // 成员函数声明
    Auto_Focusing(Motor& ir_motor1);

    int get_motor_current_pos();
    void ir_motor_move(int32_t steps);
    double get(void);
    void setxy(int x, int y);
    void set_eig(int pos);
    float get_max_eig();
    int get_max_eig_pos();
    float get_eig_max_minus_current();
    float get_eig_last_minus_current();
    void finetune(int motor_pulse_finetune);
    void show();




};

Auto_Focusing::Auto_Focusing(Motor& ir_motor1)
    :IR_Motor(ir_motor1)
    {
        
    }

int Auto_Focusing::get_motor_current_pos(){
    printf("IR_Motor.Get_Step_IR_Cur() = %d \n" , IR_Motor.Get_Step_IR_Cur());
    return IR_Motor.Get_Step_IR_Cur();
    //return static_cast<int>(IR_Motor.Get_Step_IR_Cur()); 
   // return 1;
    }
    

void Auto_Focusing::ir_motor_move(int32_t steps){
    printf("IR_Motor.Move_IR(steps) = %d \n", steps);
    IR_Motor.Move_IR(steps); 
}

void Auto_Focusing::setxy(int center_x, int center_y){
    x = center_x;
    y = center_y;
}

void Auto_Focusing::set_eig(int pos){
    if(pos>=0){
        eig = get_eig(x, y);
        if(list_index<1){
            eig_values_list[list_index] = eig;
            motor_pos_list[list_index] = current_motor_pos;
            motor_return_pos_list[list_index] = pos;
            //printf("eig_values_list[-2]= %f, list_index = %d\n", eig_values_list[list_index-1], list_index);
            list_index += 1;
        }
        else if(eig != eig_values_list[list_index-1]){
            eig_values_list[list_index] = eig;
            motor_pos_list[list_index] = current_motor_pos;
            motor_return_pos_list[list_index] = pos;
            //printf("eig_values_list[-2]= %f, list_index = %d\n", eig_values_list[list_index-1], list_index);
            list_index += 1;
        }

    }
}

float Auto_Focusing::get_max_eig(){
    return findMaxValue(eig_values_list, list_index);
}


int Auto_Focusing::get_max_eig_pos(){


    int x0 = 0;
    // int max_index = findMaxIndex(eig_values_list, list_index);
    // x0 =  motor_return_pos_list[max_index];


    // 遍历数组，找到最大值
    float max = eig_values_list[list_index-1];
    int max_index = 0;
    for (size_t i = 1; i < list_index; i++) {
        if (motor_return_pos_list[i] - motor_return_pos_list[i-1]>0) {
            if (eig_values_list[i] > max) {
                max = eig_values_list[i];
                max_index = i;
            }
        }
    }
 
    x0 =  motor_return_pos_list[max_index];

    printf("x0 = %d,  max_index = %d \n", x0,  max_index);


    // if(max_index == list_index){
    //     return x0;
    // }
    // else if(max_index > 0) {
    //     int motor_return_pos_max_points[3] = { motor_return_pos_list[max_index - 1], motor_return_pos_list[max_index], motor_return_pos_list[max_index + 1] };
    //     float eig_values_max_points[3] = { eig_values_list[max_index - 1], eig_values_list[max_index], eig_values_list[max_index + 1] };
    //     x0 = get_x0_y0(motor_return_pos_max_points, eig_values_max_points);
    //     if(x0 < 0 || x0 > IR_Motor_Step_Total) { return motor_return_pos_list[max_index];}
    // }
   
    return  x0;
}


float Auto_Focusing::get_eig_max_minus_current(){
    return findMaxValue(eig_values_list, list_index)- eig;
}

float Auto_Focusing::get_eig_last_minus_current(){
   // printf("eig_values_list[-2]= %f, eig = %f\n", eig_values_list[-2], eig);
    return eig_values_list[list_index-2] -eig ;
}


void Auto_Focusing::
finetune(int motor_pulse_finetune){

    int T = 0;
    T_cut = 0.05;
    int pos = 0;
    printf("start finetune .....\n");

    int last_list_index = list_index;
    for (int ii = 0; ii < 10; ii++) {
        ir_motor_move(motor_pulse_finetune);
        current_motor_pos += motor_pulse_finetune;

        usleep(50000);
        pos = get_motor_current_pos();
        set_eig(pos);
        if(pos <1 or pos>IR_Motor_Step_Total){
            motor_pulse_finetune *= -1;
        }
        
        printf("eig = %f, get_max_eig() = %f\n", eig, get_max_eig());
        if(eig >= get_max_eig()){ printf("max eig break\n"); break;}
       // printf("get_eig_last_minus_current() = %f\n", get_eig_last_minus_current());
        if (get_eig_last_minus_current() > T_cut) {
            motor_pulse_finetune *= -0.5;
            T += 1;
            if (T > 2) {
                ir_motor_move(motor_pulse_finetune);
                current_motor_pos += motor_pulse_finetune;
                usleep(50000);
                pos = get_motor_current_pos();

                set_eig(pos);
                break;
            }
        }

    }

}



void Auto_Focusing::
show(){
    for (size_t i = 0; i < list_index; i++) {
        printf("eig_values_list value [%d] = %f, %d, %d\n", i, eig_values_list[i], motor_pos_list[i], motor_return_pos_list[i]);
    }

}



// 第一次对焦时使用
void ir_auto_focusing_by_image_continuous(Motor& ir_motor,int x, int y) {
    printf(" auto_focusing starting .....\n");

    Auto_Focusing *auto_focusing = new Auto_Focusing(ir_motor);
    auto_focusing->setxy(x, y);

    int start_pos = 0;
    int pos = 0;
    int32_t motor_pulse = 0;

    start_pos = auto_focusing->get_motor_current_pos();
    auto_focusing->set_eig(start_pos);
    

    int count = 0;
    for(int i=0; i<3;i++)
    {
        auto_focusing->ir_motor_move(-3000);
        count = 0;
        while(count<1000)
        {
            count++;
            pos = auto_focusing->get_motor_current_pos();
            auto_focusing->set_eig(pos);
            if(pos==0) break;
            usleep(30000);
        }
        pos = auto_focusing->get_motor_current_pos();
        if(pos==0) break;
    }


        
    
    
    // 必须到0位置
    // 反转
    printf("auto_focusing->ir_motor_move(3000)\n");
    pos = auto_focusing->get_motor_current_pos();
    auto_focusing->set_eig(pos);



    bool end_move_flag = false;
    for(int i=0; i<3;i++)
    {
        printf(" bool end_move_flag = false; i= %d\n", i);
        pos = auto_focusing->get_motor_current_pos();
        motor_pulse = IR_Motor_Step_Total - pos;
        auto_focusing->ir_motor_move(motor_pulse);
        count = 0;
        while(count<1000)
        {
            count++;
            pos = auto_focusing->get_motor_current_pos();
            auto_focusing->set_eig(pos);

            // if(i <auto_focusing->list_index/2) continue;
            // if(auto_focusing->list_index/2 < abs(motor_pulse)/IR_MOTOR_MUC_RETURN_FREQ/3 ) continue;

            if(pos >1200) {end_move_flag = true;  printf("(pos >1200, end_move_flag = true; \n");}

            // if(auto_focusing->get_eig_max_minus_current() > auto_focusing->get_max_eig()/2) 
            // {
            //     printf("auto_focusing->get_eig_max_minus_current() > auto_focusing->get_max_eig()/2\n");
            //     end_move_flag = true;
            // }

            if(end_move_flag) break;
            usleep(30000);
        }
        if(end_move_flag) break;
    }


    pos = auto_focusing->get_motor_current_pos();

    int max_eig_pos = auto_focusing->get_max_eig_pos() -110;
    if(max_eig_pos<0) max_eig_pos = 0;



    end_move_flag = false;
    for(int i=0; i<3;i++)
    {
        printf(" bool end_move_flag = false; i= %d\n", i);
        pos = auto_focusing->get_motor_current_pos();
        motor_pulse = IR_Motor_Step_Total - pos;
        auto_focusing->ir_motor_move(motor_pulse);
        count = 0;
        while(count<1000)
        {
            count++;
            pos = auto_focusing->get_motor_current_pos();

            motor_pulse = max_eig_pos - pos;
            auto_focusing->ir_motor_move(motor_pulse);
            if(auto_focusing->get_max_eig() == auto_focusing->eig) {end_move_flag = true;  printf("(pos >1200, end_move_flag = true; \n");};
            if(pos <= max_eig_pos ) {end_move_flag = true;  printf("(pos >1200, end_move_flag = true; \n");};
            if(end_move_flag) break;
            usleep(30000);
        }
        if(end_move_flag) break;
    }


    pos = auto_focusing->get_motor_current_pos();

    auto_focusing->set_eig(pos);
    if(auto_focusing->get_eig_last_minus_current()>=0) motor_pulse = 20;
    else motor_pulse = -20;

    auto_focusing->finetune(motor_pulse);

    pos = auto_focusing->get_motor_current_pos();
    auto_focusing->set_eig(pos);
    
    auto_focusing->show();

    delete auto_focusing;

    //std::cout<<"end auto focusing...  "<< "IR_Motor_Sensor_Move(0) " << auto_focusing->get_motor_current_pos() << std::endl;
    // 需要获取电机当前位置
}


