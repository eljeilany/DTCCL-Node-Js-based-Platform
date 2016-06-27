#ifndef DTCCL_H
#define DTCCL_H
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <boost/container/stable_vector.hpp>
#include <boost/container/vector.hpp>
#include "fastcluster/fastcluster.h"
#include <opencv2/objdetect.hpp>
#include <opencv2/videoio.hpp>
#include <sstream>
#include <b64/encode.h>
#include <iostream>

class DTCCL
{
private:
    float thrdist ;
    float thrratio;
    float thrcutoff;
    bool class_detect;
    int dtype;

    cv::CascadeClassifier face_cascade;
    cv::Ptr<cv::FeatureDetector> detector;
    cv::Ptr<cv::DescriptorExtractor> descriptorExtractor;
    cv::Ptr<cv::DescriptorMatcher> descriptorMatcher ;

    boost::container::stable_vector<int> object_class_id;
    boost::container::stable_vector<int> background_class_id;
    boost::container::stable_vector<signed char> weight;
    std::vector<bool> matchfound;
    std::vector<bool> objectmatch;
    boost::container::stable_vector<bool> inliermatch;
    std::vector<unsigned int> selectedlearn;
    boost::container::stable_vector<int> selectedClasses;
    std::vector<unsigned int> backgroundClasses;
    std::vector<unsigned int> classesDatabase;

    unsigned int nbInitialKeypoints;
    boost::container::stable_vector<cv::Point2f> votes;
    boost::container::stable_vector<std::pair<cv::KeyPoint, int> > outliers;

    cv::Mat object_features;
    cv::Mat background_features;
    cv::Mat featuresDatabase;
    cv::Mat squareForm;
    cv::Mat angles;
    cv::Mat im_prev;

    boost::container::stable_vector<std::pair<cv::KeyPoint,int> > activeKeypoints;
    boost::container::stable_vector<std::pair<cv::KeyPoint,int> > trackedKeypoints;


    template<class T>
    int sgn(T x)
    {
        if (x >=0) return 1;
        else return -1;
    }

    void matchfoundC(unsigned int id, bool value);
    void objectmatchC(unsigned int id, bool value);
    void selectedlearnC(unsigned int id, unsigned int value);
    signed char weightf(unsigned i);



    void track(cv::Mat im_prev, cv::Mat im_gray, const boost::container::stable_vector<std::pair<cv::KeyPoint, int> >& keypointsIN,
               boost::container::stable_vector<std::pair<cv::KeyPoint, int> >& keypointsTracked,
                int THR_FB= 3);

    void match(const std::vector<cv::KeyPoint> & keypoints, const cv::Mat features,
                     boost::container::stable_vector<std::pair<cv::KeyPoint,int> >& matchedKeypoints);

    void fuse(const boost::container::stable_vector<std::pair<cv::KeyPoint,int> >& first,
              const boost::container::stable_vector<std::pair<cv::KeyPoint,int> >& second,
              boost::container::stable_vector<std::pair<cv::KeyPoint,int> >& fused) ;

    void estimateScaleRotation(const boost::container::stable_vector<std::pair<cv::KeyPoint, int> >& keypoints,
            float & scale, float & rotation);

    void Consensus(const boost::container::stable_vector<std::pair<cv::KeyPoint, int> >& keypoints,
            const float scale, const float rotation, cv::Point2f & center,
            boost::container::stable_vector<std::pair<cv::KeyPoint, int> >& inlier);
    boost::container::stable_vector<std::pair<cv::KeyPoint, int> > clusterConsensus(const boost::container::stable_vector<std::pair<cv::KeyPoint, int> > &keypointsIN,
                          const float scaleEstimate, const float medRot, cv::Point2f & center);

    void ConsensusMatch(const std::vector<cv::KeyPoint> & keypoints, const cv::Mat features,
                    const cv::Point2f center, const float scale, const float rotation,
                    boost::container::stable_vector<std::pair<cv::KeyPoint,int> >& matchedKeypoints);

    void learn(std::vector<cv::KeyPoint> Keypoints, cv::Point2f center, float scaleEstimate, float rotationEstimate,
               cv::Mat features, boost::container::stable_vector<std::pair<cv::KeyPoint,int> > inliers, boost::container::stable_vector<std::pair<cv::KeyPoint, int> > fusedKeypoints,
               boost::container::stable_vector<std::pair<cv::KeyPoint, int> > consunsusMatched, const cv::Rect rect);

public:

    cv::Size2f insize;
    cv::Point2f drawc;
    Point2f vertices[4];
    cv::Rect_<float> boundingbox;
    bool found;
    bool hasResult;

    cv::Point2f centerToTopLeft;
    cv::Point2f centerToTopRight;
    cv::Point2f centerToBottomRight;
    cv::Point2f centerToBottomLeft;

    boost::container::stable_vector<cv::Point2f> springs;
    boost::container::stable_vector<std::pair<cv::KeyPoint,int> > prvkp;
    boost::container::stable_vector<std::pair<cv::KeyPoint,int> > drawKeypoints;

    std::string serialize(cv::Mat input);
    cv::Mat deserialize(std::stringstream& input);

    DTCCL();
    void set(bool d, int dm, int kd, int km);
    void initialise(cv::Mat im_gray0, const cv::Rect rect);


    boost::container::vector< cv::Rect > detectFace(cv::Mat im_gray);
    int patchMatch(cv::Mat tpl_gray);
    boost::container::vector<cv::Rect > detectPed(cv::Mat im_gray);
    boost::container::vector< cv::Mat > detections;
    std::string justDetect(cv::Mat im_gray, std::string name);
    void processFrame(cv::Mat im_gray);
    void processFrame2(cv::Mat im_gray);
};

class Cluster
{
public:
    int first, second;//cluster id
    float dist;
    int num;
};

cv::Point2f rotate(const cv::Point2f p, const float rad);
#endif // DTCCL_H
