// Gets an image and outputs the face landmarks.
//
// Build:
//      bazel build --define MEDIAPIPE_DISABLE_GPU=1 --nocheck_visibility //src/feature_extractor:feature_extractor_single_image
//
// Run:
//      bazel-bin/src/feature_extractor/feature_extractor_single_image --input_image_path=path/to/image.jpg

#include <cstdlib>
#include <iostream>
#include <vector>
#include <cmath>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/port/commandlineflags.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

#include "src/features/face/face_analyzer.h"
#include "src/features/mouth/mouth_analyzer.h"
#include "src/features/eye/eye_analyzer.h"

DEFINE_string(input_image_path, "",
              "Path to the image.");

//
mediapipe::Status RunGraph() {

    mediapipe::CalculatorGraphConfig config = 
            mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(R"(
                input_stream: "IMAGE:input_image"
                output_stream: "LANDMARKS:multi_face_landmarks" 
                node: {
                    calculator: "FaceLandmarkFrontCpu"
                    input_stream: "IMAGE:input_image"
                    output_stream: "LANDMARKS:multi_face_landmarks"
                }
            )");

    // creates the graph with those configs
    mediapipe::CalculatorGraph graph;

    MP_RETURN_IF_ERROR(graph.Initialize(config));

    ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller,
                    graph.AddOutputStreamPoller("multi_face_landmarks"));
    
    MP_RETURN_IF_ERROR(graph.StartRun({}));

    // read input image 
    cv::Mat raw_image = cv::imread(FLAGS_input_image_path);

    // wrap cv::Mat into a ImageFrame
    auto input_frame = std::make_unique<mediapipe::ImageFrame>(
        mediapipe::ImageFormat::SRGB,
        raw_image.cols,
        raw_image.rows
    );
    
    
    cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());

    cv::cvtColor(raw_image, raw_image, cv::COLOR_BGR2RGB);

    int width = input_frame_mat.size().width;
    int height = input_frame_mat.size().height;

    raw_image.copyTo(input_frame_mat);

    // send input ImageFrame to graph
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
        "input_image",
        mediapipe::Adopt(input_frame.release())
            .At(mediapipe::Timestamp(0))
    ));

    cv::cvtColor(raw_image, raw_image, cv::COLOR_RGB2BGR);

    // gets graph output
    mediapipe::Packet output_packet;
    
    if (!poller.Next(&output_packet)) return mediapipe::OkStatus();

    // get landmark points
    auto& output_landmark_vector = 
            output_packet.Get<std::vector<mediapipe::NormalizedLandmarkList>>();

    mediapipe::NormalizedLandmarkList face_landmarks = output_landmark_vector[0];
    
    // Instantiate analyzers
    MouthAnalyzer mouth_descriptor(width, height);
    mouth_descriptor.SetLandmarks(face_landmarks);

    FaceAnalyzer face_descriptor(width, height);
    face_descriptor.SetLandmarks(face_landmarks);

    EyeAnalyzer eye_descriptor(width, height);
    eye_descriptor.SetLandmarks(face_landmarks);

    // ===========================
    // F1
    // ===========================
    double f1 = mouth_descriptor.GetMouthOuter();
    std::cout << "F1: " << f1 << std::endl;

    // ===========================
    // F2
    // ===========================
    double f2 = mouth_descriptor.GetMouthCorner();
    std::cout << "F2: " << f2 << std::endl;

    // ===========================
    // F3
    // ===========================
    double f3 = eye_descriptor.GetEyeInnerArea();
    std::cout << "F3: " << f3 << std::endl;

    // ===========================
    // F4
    // ===========================
    double f4 = eye_descriptor.GetEyebrow();
    std::cout << "F4: " << f4 << std::endl;
    
    //============================
    // F5
    // ===========================
    double f5 = face_descriptor.GetFaceArea();
    std::cout << "F5: " << f5 << std::endl;

    //============================
    // F7
    // ===========================
    double f7 = face_descriptor.GetFaceCOM();
    std::cout << "F7: " << f7 << std::endl;

    MP_RETURN_IF_ERROR(graph.CloseInputStream("input_image"));

    return graph.WaitUntilDone();
}


int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    mediapipe::Status output_status = RunGraph();

    std::cout << output_status.message() << std::endl;

    return EXIT_SUCCESS;
}