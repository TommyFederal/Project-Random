// Project gabut by TWF.

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <algorithm>

const float TEXT_FADE_DURATION = 1.5f; // Fade in timer
const float BLUR_DELAY_OFFSET = 0.0f; // pengaturan delay
const float BLUR_FADE_IN_SPEED = 1.5f;
const float BLUR_FADE_OUT_SPEED = 0.3f;


struct Heart {
    cv::Point pos;
    float vy; // velocity
    float age;
    float maxLife;
};

cv::Mat HeartImage;

void drawHeartImage(cv::Mat& frame, cv::Point pos, cv::Mat& heart, float alpha) {
    if (heart.empty()) return;

    // Pengukuran
    cv::Mat resized;
    cv::resize(heart, resized, cv::Size(40, 40));

    // Draw with ALpha blending
    for (int y = 0; y < resized.rows; y++) {
        for (int x = 0; x < resized.cols; x++) {
            int fx = pos.x + x - 20; // penengahan
            int fy = pos.y + y - 20;
            if (fx >= 0 && fx < frame.cols && fy >= 0 && fy < frame.rows) {
                cv::Vec4b pixel = resized.at<cv::Vec4b>(y, x);
                float pixelAlpha = pixel[3] / 255.0f;
                frame.at<cv::Vec3b>(fy, fx)[0] = cv::saturate_cast<uchar>(frame.at<cv::Vec3b>(fy, fx)[0] * (1 - pixelAlpha * alpha) + pixel[0] * pixelAlpha * alpha);
                frame.at<cv::Vec3b>(fy, fx)[1] = cv::saturate_cast<uchar>(frame.at<cv::Vec3b>(fy, fx)[1] * (1 - pixelAlpha * alpha) + pixel[1] * pixelAlpha * alpha);
                frame.at<cv::Vec3b>(fy, fx)[2] = cv::saturate_cast<uchar>(frame.at<cv::Vec3b>(fy, fx)[2] * (1 - pixelAlpha * alpha) + pixel[2] * pixelAlpha * alpha);
            }
        }
    }
}

int main() {
    std::vector<Heart> hearts;
    cv::VideoCapture cap(0); // 0 = Webcam Perangkat
    cv::Mat frame, hsv, skinMask;
    float blurIntensity = 0.f;
    float blurTimer = 0.f;
    int fingerCount = 0;
    HeartImage = cv::imread("C:/Users/ASUS/Documents/Random Script/heart.png", cv::IMREAD_UNCHANGED);
    // Posisi Pops up
    cv::namedWindow("Foto Kita Blur", cv::WINDOW_NORMAL);
    cv::resizeWindow("Foto Kita Blur", 800, 600);
    cv::moveWindow("Foto Kita Blur", 100, 100);
    if (!cap.isOpened()) {
        std::cerr << "Webcam tidak ditemukan.\n";
        return 1;
    
    }

    while (true) {
        static cv::TickMeter timer;
        timer.start();

        cap >> frame;
        if (frame.empty()) break;

        timer.stop();
        float dt = timer.getTimeSec();
        timer.reset();

        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

        // inRange untuk semua pixel antar lower & upper
        skinMask = cv::Mat::zeros(frame.size(), CV_8U);
        cv::Scalar lower1 = cv::Scalar(0, 60, 160); // H, S, V Min
        cv::Scalar upper1 = cv::Scalar(8, 90, 255);

        cv::Scalar lower2 = cv::Scalar(172, 60, 160);
        cv::Scalar upper2 = cv::Scalar(180, 90, 255);

        cv::Mat mask1, mask2;
        cv::inRange(hsv, lower1, upper1, mask1); // pencarian range 1
        cv::inRange(hsv, lower2, upper2, mask2); // pencarian range2
        cv::bitwise_or(mask1, mask2, skinMask);

        //Check if tangan di atas frame
        cv::Mat upperHalf = skinMask(cv::Rect(0, 0, skinMask.cols, skinMask.rows / 2));
        int whitePixels = cv::countNonZero(upperHalf);

        if (whitePixels > 12000) {
            blurIntensity += dt * BLUR_FADE_IN_SPEED;
            if (blurIntensity > 1.0f) blurIntensity = 1.0f;
        } else {
            blurIntensity -= dt * BLUR_FADE_OUT_SPEED;
            if (blurIntensity < 0.f) blurIntensity = 0.f;
        }

        // Munculin Hati waktu ngeblur
        if (blurIntensity > 0.5f && hearts.size() < 50) {
            Heart h;
            h.pos = cv::Point(50 + (std::rand() % (frame.cols - 100)), frame.rows - 50);
            h.vy = -(50 + std::rand() % 100);
            h.age = 0.f;
            h.maxLife = 2.0f;
            hearts.push_back(h);
        }

        // Penggambaran
        for (auto& h : hearts) {
            h.age += dt;
            h.pos.y += h.vy * dt;

            float lifeFactor = 1.0f - (h.age / h.maxLife);
            if (lifeFactor < 0.f) lifeFactor = 0.f;

            drawHeartImage(frame, h.pos, HeartImage, lifeFactor);
        }
        hearts.erase(std::remove_if(hearts.begin(), hearts.end(), [](const Heart& h) { return h.age >= h.maxLife; }), hearts.end());

        // Pengaplikasian blur based pada blurIntensity
        if (blurIntensity > 0.0f) {
            cv::Mat blurred;
            cv::GaussianBlur(frame, blurred, cv::Size(51, 51), 0);
            cv::addWeighted(blurred, blurIntensity, frame, 1.0f - blurIntensity, 0, frame);
        }

        // peletakan center
        float textAlpha = blurIntensity;
        if (textAlpha > 0.1f) {
            cv::Mat textOverlay = frame.clone();
            cv::putText(textOverlay, "Foto Kita Blurrr", cv::Point(frame.cols/2 - 150, frame.rows/2), cv::FONT_HERSHEY_SIMPLEX, 2.0, cv::Scalar(255, 255, 255), 3);
            cv::addWeighted(textOverlay, textAlpha, frame, 1.0 - textAlpha, 0, frame);
        }
        
        cv::imshow("Foto Kita Blur", frame);
        // cv::imshow("Skin Mask", skinMask); saya matikan biar kalau kebuka ga ngagetin.
        
        if (cv::waitKey(1) == 27) break;
    }

    return 0;
}