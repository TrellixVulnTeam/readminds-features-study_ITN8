// Copyright 2021 The authors

#ifndef SRC_FEATURES_GENERIC_ANALYZER_H_
#define SRC_FEATURES_GENERIC_ANALYZER_H_

#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/port/opencv_core_inc.h"

const int NTOTAL_LANDMARKS = 468;

const int ANCHOR_LANDMARKS[] = {
    1, 4, 5, 195, 197, 6,
};

// Face object providing generic data used by
// other features, such as A and K.
class GenericAnalyzer {
    public:
        GenericAnalyzer() = default;

        GenericAnalyzer(int img_width, int img_height);

        GenericAnalyzer(mediapipe::NormalizedLandmarkList landmarks,
                     int img_width, int img_height);

        virtual ~GenericAnalyzer() = default;

        // landmarks_ setter
        void SetLandmarks(mediapipe::NormalizedLandmarkList landmarks);

        // Sets all needed attributes
        void Initialize(int img_width, int img_height);

        void Initialize(mediapipe::NormalizedLandmarkList landmarks,
                        int img_width, int img_height);

    protected:
        double norm_factor_;

        mediapipe::NormalizedLandmarkList landmarks_;
        int img_width_;
        int img_height_;

        // Implements the euclidean distance between
        // two cv::Points
        double EuclideanDistance(cv::Point a, cv::Point b);

        double EuclideanDistance(double x1, double y1, double x2, double y2);

        // Updates the normalize factor with landmarks value
        void CalculateNormFactor();

        // Pure virtual function responsible for updating all
        // feature values within an analyzer.
        virtual void Update() = 0;

        // Converts a normalized landmark coordinate into
        // a OpenCV point. Depth (z) is not been taken
        // into account (yet).
        cv::Point CvtNormIntoCvPoint(mediapipe::NormalizedLandmark landmark);

        // Computes the eucliand norm for
        // a given coordinate
        double EuclideanNorm(cv::Point landmark);
};

#endif  // SRC_FEATURES_GENERIC_ANALYZER_H_
