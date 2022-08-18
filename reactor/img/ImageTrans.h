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
public:
    cv::Mat m;
    int D=50;

    std::vector<char>c={
            '@','#','G','O','s','c',':',',','.',
    };

    vector<vector<char>> loadpng(const std::vector<char>& file){
        m = imdecode(file, cv::IMREAD_COLOR );
        std::cout<<"row col: "<<m.rows<<" "<<m.cols<<std::endl;
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
            ret.push_back(vector<char>());
            for(int j=0; j<m.cols; j++){
                if(m.at<uchar>(i, j)* c.size() / 256 >= c.size())
                    cout<<endl<<endl;
                ret[i].push_back( c[ m.at<uchar>(i, j)* c.size() / 256 ]);
                cout<<ret[i].back();
            }
            cout<<endl;
        }
        cout<<m.rows<<" "<<m.cols<<endl;
        return ret;
    }
};


#endif //TOYWEBSERVER_IMAGETRANS_H
