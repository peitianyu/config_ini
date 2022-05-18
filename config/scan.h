#ifndef _SCAN_H_
#define _SCAN_H_

#include<string.h>
#include<stdio.h>
#include<stdint.h>
#include<vector>

#include <eigen3/Eigen/Core>
#include <opencv2/opencv.hpp>
#include "CFG_ini.h"
#include "DataPointContainer.h"

namespace sensor{

class LaserScan{
public:
    LaserScan(){}
    ~LaserScan(){}
    LaserScan(const double &_angle_min, const double &_angle_max, const double &_angle_increment, const double &_range_min, const double &_range_max,const int &_range_size):
        angle_min(_angle_min), angle_max(_angle_max), angle_increment(_angle_increment), range_min(_range_min), range_max(_range_max), range_size(_range_size){
        time_stamp = 0;
        dataContainer.setOrigo(Eigen::Vector2f(0,0));
    }
    
    LaserScan(const char* ConfigFile_name)
    {
        Config *USINGINFOPAHT;
        USINGINFOPAHT = cnf_read_config(ConfigFile_name, '#', ':');
        cnf_get_value(USINGINFOPAHT, "scan", "angle_min");
        angle_min = USINGINFOPAHT->re_double;
        cnf_get_value(USINGINFOPAHT, "scan", "angle_max");
        angle_max = USINGINFOPAHT->re_double;
        cnf_get_value(USINGINFOPAHT, "scan", "angle_increment");
        angle_increment = USINGINFOPAHT->re_double;
        cnf_get_value(USINGINFOPAHT, "scan", "range_min");
        range_min = USINGINFOPAHT->re_double;
        cnf_get_value(USINGINFOPAHT, "scan", "range_max");
        range_max = USINGINFOPAHT->re_double;
        cnf_get_value(USINGINFOPAHT, "scan", "range_size");
        range_size = USINGINFOPAHT->re_int;
        time_stamp = 0;
        dataContainer.setOrigo(Eigen::Vector2f(0,0));
    }

    const int size()
	{
		return range_size;
	}

    void laserData2Container()
    {
        float angle = this->angle_min;
        float range_max = this->range_max - 0.1f;
        for( int i = 0; i < this->range_size; i ++ ){
            float dist = this->ranges[i];
    
            if( ( dist > this->range_min ) && ( dist < range_max ) ){
                float x = dist * cos( angle );
                float y = dist * sin( angle );
                dataContainer.add( Eigen::Vector2f( x, y ));
            }
            angle += this->angle_increment;
        }
    }

    void displayAFrameScan(float scale ,uint32_t window_size)
    {
        cv::Mat image = cv::Mat::zeros( window_size, window_size, CV_8UC3 );
        cv::Point2d center( window_size/2, window_size/2 );
        cv::circle(image, center, 1, cv::Scalar(0, 255, 0), 1);
        cv::line( image, cv::Point( window_size/2, 0 ), cv::Point( window_size/2, window_size ), cv::Scalar( 67, 128, 94 ), 1 );
        cv::line( image, cv::Point( 0, window_size/2 ), cv::Point( window_size, window_size/2 ), cv::Scalar( 67, 128, 94 ), 1 );	

        for( auto it : dataContainer.getDataPoints() ){
            cv::Point2d point( it(0) * scale + window_size/2, it(1) * scale + window_size/2 );
            cv::circle(image, point, 3, cv::Scalar(0, 0, 255), 1);
        } 
        cv::imshow( "scan", image );
        cv::waitKey(1);
    }

public:
    uint32_t time_stamp;
    
    std::vector<float> ranges;
    std::vector<float> intensities;
    DataContainer dataContainer;

private:
    float angle_min;
    float angle_max;
    float angle_increment;
    float range_min;
    float range_max;
    int range_size;

}; // class LaserScan

} // namespace sensor

#endif