#include "extract_cow_number.hpp"

#include <ocv_libs/core/attribute.hpp>

#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/text.hpp>

#include <iostream>

/**
 *solve problem post at https://stackoverflow.com/questions/29923827/extract-cow-number-from-image
 */

using namespace std;
using namespace cv;

using cpoints = vector<Point>;

namespace{

cv::Mat fore_ground_extract(cv::Mat const &input)
{
    vector<Mat> bgr;
    split(input, bgr);

    //process on blue channel as andrew suggest, because it is
    //easier to get rid of vegetation
    Mat text_region = bgr[0];
    blur(text_region, text_region, {3, 3});
    threshold(text_region, text_region, 0, 255, cv::THRESH_OTSU);
    medianBlur(text_region, text_region, 5);
    Mat const erode_kernel = getStructuringElement(MORPH_ELLIPSE, {11, 11});
    erode(text_region, text_region, erode_kernel);
    Mat const dilate_kernel = getStructuringElement(MORPH_ELLIPSE, {7, 7});
    dilate(text_region, text_region, dilate_kernel);
    bitwise_not(text_region, text_region);

    return text_region;
}

std::vector<std::vector<cv::Point>> get_text_contours(cv::Mat const &input)
{
    //Try ERFilter of opencv if accuracy of this solution is low
    vector<cpoints> contours;
    findContours(input, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    auto outlier = [](cpoints const &cp)
    {
        auto const area = cv::boundingRect(cp).area();

        return area < 900 || area >= 10000;
    };
    auto it = std::remove_if(std::begin(contours), std::end(contours), outlier);
    contours.erase(it, std::end(contours));//*/

    std::sort(std::begin(contours), std::end(contours), [](cpoints const &lhs, cpoints const &rhs)
    {

        return cv::boundingRect(lhs).x < cv::boundingRect(rhs).x;
    });

    return contours;
}

}

void extract_cow_number()
{
    Mat input = imread("../forum_quest/data/cow_02.jpg");
    cout<<input.size()<<endl;
    if(input.empty()){
        cerr<<"cannot open image\n";
        return;
    }
    if(input.cols > 1000){
        cv::resize(input, input, {1000, (int)(1000.0/input.cols * input.rows)}, 0.25, 0.25);
    }

    Mat crop_region;
    input(Rect(0, 0, input.cols, input.rows/3)).copyTo(crop_region);

    Mat const text_region = fore_ground_extract(crop_region);
    vector<cpoints> const text_contours = get_text_contours(text_region);
    RNG rng(12345);
    Mat text_mask(text_region.size(), CV_8UC3);

    string const vocabulary = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; // must have the same order as the classifier output classes
    Ptr<text::OCRHMMDecoder::ClassifierCallback> ocr = text::loadOCRHMMClassifierCNN("OCRBeamSearch_CNN_model_data.xml.gz");
    vector<int> out_classes;
    vector<double> out_confidences;
    for(size_t i = 0; i < text_contours.size(); ++i){
        Scalar const color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours(text_mask, text_contours, static_cast<int>(i), color, 2);
        auto const text_loc = boundingRect(text_contours[i]);
        //crop_region can gain highest accuracy
        rectangle(crop_region, text_loc, color, 2);
        ocr->eval(crop_region(text_loc), out_classes, out_confidences);
        cout << "OCR output = \"" << vocabulary[out_classes[0]]
                << "\" with confidence "
                << out_confidences[0] << std::endl;
        //ocv::print_contour_attribute(text_contours[i], 0.001, cout);
        imshow("text_mask", text_mask);
        imshow("crop_region", crop_region(text_loc));
        waitKey();
    }

    imshow("crop", crop_region);
    imshow("text_region", text_region);
    imshow("text mask", text_mask);
    waitKey();
}
