//
// Created by Clouver on 2022/8/17.
//

#ifndef TOYWEBSERVER_IMAGETRANS_H
#define TOYWEBSERVER_IMAGETRANS_H

#include<iostream>
#include<opencv4/opencv2/core/core.hpp>
#include<opencv4/opencv2/highgui.hpp>
#include<opencv4/opencv2/opencv.hpp>
#include<opencv4/opencv2/core/mat.hpp>
#include<opencv4/opencv2/imgproc/imgproc.hpp>

using namespace std;

class ImageTrans{
    cv::Mat m;
    int D=50;

    std::vector<char>c={
            '@','#','G','O','s','c',':',',','.',
    };

public:
    vector<vector<char>> loadpng(const std::vector<char>& file);
};


#endif //TOYWEBSERVER_IMAGETRANS_H
