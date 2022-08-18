//
// Created by Clouver on 2022/8/17.
//

#include "ImageTrans.h"


vector<vector<char>> ImageTrans::loadpng(const std::vector<char>& file){
    m = imdecode(file, cv::IMREAD_COLOR );
    int newM, newN;
    double r = 1.0*m.rows/m.cols;
    if(m.rows > m.cols){
        newM = D;
        newN = newM / r;
    }
    else{
        newN = D;
        newM = newN *r;
    }
    resize(m, m, cv::Size(newN,newM), 0, 0, cv::INTER_LINEAR);
    cvtColor(m,m,cv::COLOR_BGR2GRAY);
    vector<vector<char>>ret;
    for(int i=0; i<m.rows; i++){
        ret.emplace_back();
        for(int j=0; j<m.cols; j++){
            ret[i].push_back( c[ m.at<uchar>(i, j)* c.size() / 256 ]);
        }
    }
    return ret;
}