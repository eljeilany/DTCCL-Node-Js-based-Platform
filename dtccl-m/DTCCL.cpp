#include "DTCCL.h"
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <cmath>



#if __cplusplus < 201103L //test if c++11

    #include <limits>

    #ifndef NAN
    //may not be correct on all compilator, DON'T USE the flag FFAST-MATH

        #define NAN std::numeric_limits<float>::quiet_NaN()

        template <T>
        bool isnan(T d)
        {
          return d != d;
        }
    #endif

#endif


//Template fuctions and datatypes

typedef std::pair<int,int> PairInt;
typedef std::pair<float,int> PairFloat;

template<typename T>
bool comparatorPair ( const std::pair<T,int>& l, const std::pair<T,int>& r)
{
    return l.first < r.first;
}

template<typename T>
bool comparatorPairDesc ( const std::pair<T,int>& l, const std::pair<T,int>& r)
{
    return l.first > r.first;
}

template <typename T>
T sign(T t)
{
    if( t == 0 )
        return T(0);
    else
        return (t < 0) ? T(-1) : T(1);
}

template<typename T>
T median(std::vector<T> list)
{
    T val;
    std::nth_element(&list[0], &list[0]+list.size()/2, &list[0]+list.size());
    val = list[list.size()/2];
    if(list.size()%2==0)
    {
        std::nth_element(&list[0], &list[0]+list.size()/2-1, &list[0]+list.size());
        val = (val+list[list.size()/2-1])/2;
    }
    return val;
}

float median2(std::vector<float> & A)
{

    if (A.size() == 0)
    {
        return numeric_limits<float>::quiet_NaN();
    }

    std::nth_element(A.begin(), A.begin() + A.size()/2, A.end());

    return A[A.size()/2];
}

void fcluster_rec(boost::container::stable_vector<int>& data, const boost::container::stable_vector<Cluster>& clusters, float threshold, const Cluster& currentCluster, int& binId){
    int startBin = binId;
    if(currentCluster.first >= (int)data.size())
        fcluster_rec(data, clusters, threshold, clusters[currentCluster.first - data.size()], binId);
    else data[currentCluster.first] = binId;

    if(startBin == binId && currentCluster.dist >= threshold)
        binId++;
    startBin = binId;

    if(currentCluster.second >= (int)data.size())
        fcluster_rec(data, clusters, threshold, clusters[currentCluster.second - data.size()], binId);
    else data[currentCluster.second] = binId;

    if(startBin == binId && currentCluster.dist >= threshold)
        binId++;
}

boost::container::stable_vector<int> fcluster(const boost::container::stable_vector<Cluster>& clusters, float threshold){
    boost::container::stable_vector<int> data;
    for(unsigned int i = 0; i < clusters.size()+1; i++)
        data.push_back(0);
    int binId = 0;
    fcluster_rec(data, clusters, threshold, clusters[clusters.size()-1], binId);
    return data;
}

float findMinSymetric(const boost::container::stable_vector<boost::container::stable_vector<float> >& dist, const boost::container::stable_vector<bool>& used, int limit, int &i, int &j){
    float min = dist[0][0];
    i = 0;
    j = 0;
    for(int x = 0; x < limit; x++)
    {
        if(!used[x])
        {
            for(int y = x+1; y < limit; y++)
                if(!used[y] && dist[x][y] <= min)
                {
                    min = dist[x][y];
                    i = x;
                    j = y;
                }
        }
    }
    return min;
}

boost::container::stable_vector<Cluster> linkage(const boost::container::stable_vector<cv::Point2f>& list){

    float inf = 10000000.0;
    boost::container::stable_vector<bool> used;

    for(unsigned int i = 0; i < 2*list.size(); i++)
        used.push_back(false);

    boost::container::stable_vector<boost::container::stable_vector<float> > dist;
    for(unsigned int i = 0; i < list.size(); i++)
    {
        boost::container::stable_vector<float> line;
        for(unsigned int j = 0; j < list.size(); j++)
        {
            if(i == j)
                line.push_back(inf);
            else
            {
                cv::Point2f p = list[i]-list[j];
                line.push_back(sqrt(p.dot(p)));
            }
        }
        for(unsigned int j = 0; j < list.size(); j++)
            line.push_back(inf);
        dist.push_back(line);
    }
    for(unsigned int i = 0; i < list.size(); i++)
    {
        boost::container::stable_vector<float> line;
        for(unsigned int j = 0; j < 2*list.size(); j++)
            line.push_back(inf);
        dist.push_back(line);
    }
    boost::container::stable_vector<Cluster> clusters;
    while(clusters.size() < list.size()-1)
    {
        int x, y;
        float min = findMinSymetric(dist, used, list.size()+clusters.size(), x, y);
        Cluster cluster;
        cluster.first = x;
        cluster.second = y;
        cluster.dist = min;
        cluster.num = (x < (int)list.size() ? 1 : clusters[x-list.size()].num) + (y < (int)list.size() ? 1 : clusters[y-list.size()].num);
        used[x] = true;
        used[y] = true;
        int limit = list.size()+clusters.size();
        for(int i = 0; i < limit; i++)
        {
            if(!used[i])
                dist[i][limit] = dist[limit][i] = std::min(dist[i][x], dist[i][y]);
        }
        clusters.push_back(cluster);
    }
    return clusters;
}


// The class constructor
DTCCL::DTCCL()
{

}

void DTCCL::set(bool d, int dm, int kd, int km)
{

    //matcherType = "BruteForce-Hamming";
    switch (kd) {
    case 0:
        detector =  cv::xfeatures2d::SIFT::create() ;
        descriptorExtractor =  cv::xfeatures2d::SIFT::create() ;
        break;
    case 1:
        detector =  cv::xfeatures2d::SURF::create() ;
        descriptorExtractor =  cv::xfeatures2d::SURF::create() ;
        break;
    case 2:
        detector =  cv::AKAZE::create() ;
        descriptorExtractor =  cv::AKAZE::create() ;
        break;
    case 3:
        detector =  cv::BRISK::create() ;
        descriptorExtractor =  cv::BRISK::create() ;
        break;
    case 4:
        detector =  cv::ORB::create() ;
        descriptorExtractor =  cv::ORB::create() ;
        break;
    default:
        detector =  cv::xfeatures2d::SIFT::create() ;
        descriptorExtractor =  cv::xfeatures2d::SIFT::create() ;
        break;
    }

    switch (km) {
    case 0:
        descriptorMatcher = cv::DescriptorMatcher::create("BruteForce");
        break;
    case 1:
        descriptorMatcher = cv::DescriptorMatcher::create("BruteForce-L1");
        break;
    case 2:
        descriptorMatcher = cv::DescriptorMatcher::create("BruteForce-Hamming");
        break;
    case 3:
        descriptorMatcher = cv::DescriptorMatcher::create("BruteForce-Hamming(2)");
        break;
    case 4:
        descriptorMatcher = cv::DescriptorMatcher::create("FlannBased");
        break;
    default:
        descriptorMatcher = cv::DescriptorMatcher::create("BruteForce-L1");
        break;
    }

    if(d){
    this->class_detect = d;
    this->dtype = dm;
    }

    nbInitialKeypoints = 0;

    thrdist = 0.2;
    thrratio = 0.8;
    thrcutoff = 15;
    /** intialize the detector, extrator and matcher **/

    //descriptorMatcher = cv::DescriptorMatcher::create("BruteForce-Hamming") ;
}

/** Support Functions **/
// This function destribute keypoints between object class  and back ground class
void inout_rect(const boost::container::stable_vector<cv::KeyPoint>& keypoints, cv::Point2f topleft, cv::Point2f bottomright, boost::container::stable_vector<cv::KeyPoint>& in, boost::container::stable_vector<cv::KeyPoint>& out){
    for(unsigned int i = 0; i < keypoints.size(); i++)
    {
        if(keypoints[i].pt.x > topleft.x && keypoints[i].pt.y > topleft.y && keypoints[i].pt.x < bottomright.x && keypoints[i].pt.y < bottomright.y)
            in.push_back(keypoints[i]);
        else out.push_back(keypoints[i]);
    }
}


// This function rotates a cv point p by angle rad
cv::Point2f rotate(const cv::Point2f p,const float rad){
    if(rad == 0)
        return p;
    float s = sin(rad);
    float c = cos(rad);
    return cv::Point2f(c*p.x-s*p.y,s*p.x+c*p.y);
}



boost::container::stable_vector<int> binCount(const boost::container::stable_vector<int>& T) {
    boost::container::stable_vector<int> result;
    for(unsigned int i = 0; i < T.size(); i++)
    {
        while(T[i] >= (int)result.size())
            result.push_back(0);
        result[T[i]]++;
    }
    return result;
}



int argmax(const boost::container::stable_vector<int>& list) {
    int max = list[0];
    int id = 0;
    for(unsigned int i = 1; i < list.size(); i++)
        if(list[i] > max)
        {
            max = list[i];
            id = i;
        }
    return id;
}

boost::container::stable_vector<bool> in1d(const boost::container::stable_vector<int>& a, const boost::container::stable_vector<int>& b) {
    boost::container::stable_vector<bool> result;
    for(unsigned int i = 0; i < a.size(); i++)
    {
        bool found = false;
        for(unsigned int j = 0; j < b.size(); j++)
            if(a[i] == b[j])
            {
                found = true;
                break;
            }
        result.push_back(found);
    }
    return result;
}

void DTCCL::matchfoundC(unsigned int id, bool value){
    if(id <= matchfound.size()){
    DTCCL::matchfound[id] = value ;
    }
}

void DTCCL::objectmatchC(unsigned int id, bool value){
    if(id <= DTCCL::objectmatch.size()){
    DTCCL::objectmatch[id] = value ;
    }
}

void DTCCL::selectedlearnC(unsigned int id, unsigned int value){
    if(id <= DTCCL::selectedlearn.size()){
    DTCCL::selectedlearn[id] = value ;
    }
}


/** Extra **/
std::string DTCCL::serialize(cv::Mat input)
{
    // We will need to also serialize the width, height, type and size of the matrix
    int width = input.cols;
    int height = input.rows;
    int type = input.type();
    size_t size = input.total() * input.elemSize();

    // Initialize a stringstream and write the data
    std::stringstream ss;
    ss.write((char*)(&width), sizeof(int));
    ss.write((char*)(&height), sizeof(int));
    ss.write((char*)(&type), sizeof(int));
    ss.write((char*)(&size), sizeof(size_t));

    // Write the whole image data
    ss.write((char*)input.data, size);

    return ss.str();
}

// Deserialize a Mat from a stringstream
cv::Mat DTCCL::deserialize(std::stringstream& input)
{
    std::cout << "de s start" << std::endl;
    // The data we need to deserialize
    int width = 0;
    int height = 0;
    int type = 0;
    size_t size = 0;
    std::cout << "de s alloc" << std::endl;
    // Read the width, height, type and size of the buffer
    //std::cout << input.str() << std::endl;
    input.read((char*)(&width), sizeof(int));
    input.read((char*)(&height), sizeof(int));
    input.read((char*)(&type), sizeof(int));
    input.read((char*)(&size), sizeof(size_t));
    std::cout << "de s Allocate a buffer w x h  "<< width <<" x "<< height << " s "
    << size <<" type" << type << std::endl;
    // Allocate a buffer for the pixels
    char* data = new char[size];
    // Read the pixels from the stringstream
    input.read(data, size);
    std::cout << "de s data read" << std::endl;
    // Construct the image (clone it so that it won't need our buffer anymore)
    cv::Mat m = cv::Mat(height, width, type, data).clone();

    // Delete our buffer
    delete[]data;

    // Return the matrix
    std::cout << "de s end" << std::endl;
    return m;
}

void fastMatchTemplate(cv::Mat& srca,  // The reference image
                       cv::Mat& srcb,  // The template image
                       cv::Mat& dst,   // Template matching result
                       int maxlevel)   // Number of levels
{
    std::vector<cv::Mat> refs, tpls, results;

    // Build Gaussian pyramid
    cv::buildPyramid(srca, refs, maxlevel);
    cv::buildPyramid(srcb, tpls, maxlevel);

    cv::Mat ref, tpl, res;

    // Process each level
    for (int level = maxlevel; level >= 0; level--)
    {
        ref = refs[level];
        tpl = tpls[level];
        res = cv::Mat::zeros(ref.size() + cv::Size(1,1) - tpl.size(), CV_32FC1);

        if (level == maxlevel)
        {
            // On the smallest level, just perform regular template matching
            cv::matchTemplate(ref, tpl, res, CV_TM_CCORR_NORMED);
        }
        else
        {
            // On the next layers, template matching is performed on pre-defined
            // ROI areas.  We define the ROI using the template matching result
            // from the previous layer.

            cv::Mat mask;
            cv::pyrUp(results.back(), mask);

            cv::Mat mask8u;
            mask.convertTo(mask8u, CV_8U);

            // Find matches from previous layer
            std::vector<std::vector<cv::Point> > contours;
            cv::findContours(mask8u, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

            // Use the contours to define region of interest and
            // perform template matching on the areas
            for (unsigned i = 0; i < contours.size(); i++)
            {
                cv::Rect r = cv::boundingRect(contours[i]);
                cv::matchTemplate(
                    ref(r + (tpl.size() - cv::Size(1,1))),
                    tpl,
                    res(r),
                    CV_TM_CCORR_NORMED
                );
            }
        }

        // Only keep good matches
        cv::threshold(res, res, 0.94, 1., CV_THRESH_TOZERO);
        results.push_back(res);
    }

    res.copyTo(dst);
}

int DTCCL::patchMatch(cv::Mat tpl_gray){
    cv::Mat dst;
    double minval, maxval, maxv;
    cv::Point minloc, maxloc;
    maxv = -1.0;
    int id = -1;
    for(size_t i=0; i<detections.size(); i++){

        fastMatchTemplate(detections[i], tpl_gray, dst, 2);
        cv::minMaxLoc(dst, &minval, &maxval, &minloc, &maxloc);
        if(maxv < maxval){
        id = i;
        maxv=maxval;
        }
    }

    if(maxv > 0.45){
    return id;
    }else{
    return -1;
    }
}

std::string DTCCL::justDetect(cv::Mat im_gray, std::string name){
    std::stringstream ss;
    base64::encoder E;
    std::cout << "b det1" << std::endl;
    boost::container::vector< cv::Rect > det = detectFace(im_gray);
    std::cout << "det numb: "<< det.size() << std::endl;
    ss << "{ \"name\" :\""<< name <<"\", \"detections\":[ ";
    bool first = true;
    for(unsigned i=0; i<det.size(); i++){
        cv::Rect rect = det[i];
        // std::cout << "rect x y "<< rect.x << " " << rect.y << std::endl;
        // std::cout << "rect w h "<< rect.width << " " << rect.height << std::endl;
        // std::cout << "mat w h " << im_gray.rows << " " << im_gray.cols << std::endl;
        // cv::Mat tlp_g = im_gray(rect);
        // std::cout << "patch w h " << tlp_g.rows << " " << tlp_g.cols << std::endl;
        // std::cout << "mat after w h " << im_gray.rows << " " << im_gray.cols << std::endl;
        // cv::Mat tlp_s;
        // cv::resize(tlp_g,tlp_s,cv::Size(40,40));
        // int id = patchMatch(tlp_s);
        // std::stringstream imaged;
        // imaged << serialize(tlp_g);
        // std::stringstream image;
        // E.encode(imaged, image);
        // if(id == -1){
        //     detections.push_back(tlp_s);
            if(first){
                ss << "{ \"name\":\"" << name <<"\", \"x\":\""<< rect.x
                <<"\", \"y\":\""<<rect.y<<"\", \"height\":\""<<rect.height<<"\", \"width\":\""<<rect.width
                <<"\"}";
                first = false;
            }else{
                ss << ",{ \"name\":\"" << name <<"\", \"x\":\""<< rect.x
                <<"\", \"y\":\""<<rect.y<<"\", \"height\":\""<<rect.height<<"\", \"width\":\""<<rect.width
                <<"\"}";
            }
        // }else{
        //   std::cout << "match found" << std::endl;
        // }
    }
    ss << "] }";
    return ss.str();
}
/** The Actual Algorithm Functions **/
// The Initialization procedure

void DTCCL::initialise(cv::Mat im_gray0, const cv::Rect rect) {

    std::cout << "good phase1" << std::endl;
    insize = rect.size();
    //Get initial keypoints in whole image
    std::vector<cv::KeyPoint> keypoints;

    //detector= cv::BRISK::create() ;
    //descriptorExtractor= cv::BRISK::create() ;

    // Detect keypoints
    detector->detect(im_gray0, keypoints);

    std::cout << "good phase2" << std::endl;

    // Initialise object class
    std::vector<cv::KeyPoint> object_class_kps;
    // Initialise background class
    std::vector<cv::KeyPoint> background_class_kps;

    // Distribute keypoints between object class and back ground class
    for (size_t i = 0; i < keypoints.size(); i++){
        cv::KeyPoint kp = keypoints[i];
        cv::Point2f pt = kp.pt;
        if ( rect.contains(pt) ){
            object_class_kps.push_back(kp);
        } else {
            background_class_kps.push_back(kp);
        }
    }

    // Check if the object class is empty
    if(object_class_kps.size() == 0)
    {
        printf("No keypoints found in selection");
        return;
    }

    // Computing object class descriptors
    descriptorExtractor->compute(im_gray0, object_class_kps, object_features);

    // Computing object background descriptors
    descriptorExtractor->compute(im_gray0, background_class_kps, background_features);

    std::cout << "good phase3" << std::endl;

    // Assign each keypoint in objetc to a class
    for (size_t i = 0; i < object_class_kps.size(); i++)
    {
        object_class_id.push_back(i);
        weight.push_back(-1);
    }

    // Only now is the right time to convert keypoints to points, as compute() might remove some keypoints
    boost::container::stable_vector<cv::Point2f> object_points;
    boost::container::stable_vector<cv::Point2f> bg_points;

    for (size_t i = 0; i < object_class_kps.size(); i++)
    {
        object_points.push_back(object_class_kps[i].pt);
    }

    // Compute center of rect
    cv::Point2f center = cv::Point2f(rect.x + rect.width/2.0, rect.y + rect.height/2.0);

    // Calculate springs of each keypoint
    springs = boost::container::stable_vector<cv::Point2f>();
    for(unsigned int i = 0; i < object_class_kps.size(); i++)
        springs.push_back(object_points[i] - center);

    // Stack background features and object features into database
    cv::vconcat(background_features,object_features,featuresDatabase);

    // Initialise BG class
    backgroundClasses = std::vector<unsigned int>(background_features.rows,0);

    // Initialise classes database
    classesDatabase.clear();
    classesDatabase = backgroundClasses;
    classesDatabase.insert(classesDatabase.end(),object_class_id.begin(),object_class_id.end());

    //Get all distances between selected keypoints in squareform and get all angles between selected keypoints
    size_t num_points = springs.size();

    //Create matrices of pairwise distances/angles
    squareForm = cv::Mat(num_points, num_points, CV_32FC1);
    angles = cv::Mat(num_points, num_points, CV_32FC1);

    for (size_t i = 0; i < num_points; i++)
    {
        for (size_t j = 0; j < num_points; j++)
        {
            cv::Point2f v = springs[i] - springs[j];

            float distance = cv::norm(v);
            float angle = std::atan2(v.y,v.x);

            squareForm.at<float>(i,j) = distance;
            angles.at<float>(i,j) = angle;
        }

    }

    //Set start image for tracking
    im_prev = im_gray0.clone();

    //Make keypoints 'active' keypoints
    activeKeypoints = boost::container::stable_vector<std::pair<cv::KeyPoint,int> >();
    for(unsigned int i = 0; i < object_class_kps.size(); i++)
        activeKeypoints.push_back(std::make_pair(object_class_kps[i], i)); //object_class_id[i]));

    //Remember number of initial keypoints
    nbInitialKeypoints = object_class_kps.size();

    //Initialise weights
    for(unsigned int i = 0; i < object_class_id.size(); i++){
        weight[object_class_id[i]]+= 10;
    }
    trackedKeypoints = activeKeypoints;
    ///////////////////
    /*
    //Assign each keypoint a class starting from 1, background is 0
    {
    selectedClasses = std::vector<int>();
    for(unsigned int i = 1; i <= object_class_kps.size(); i++)
        selectedClasses.push_back(i);

    backgroundClasses = std::vector<int>();
    for(unsigned int i = 0; i < background_class_kps.size(); i++)
        backgroundClasses.push_back(0);
    }

    //Stack background features and selected features into database
    featuresDatabase = cv::Mat(background_features.rows+object_features.rows, std::max(background_features.cols,object_features.cols), background_features.type());

    if(background_features.cols > 0)
        background_features.copyTo(featuresDatabase(cv::Rect(0,0,background_features.cols, background_features.rows)));
    if(object_features.cols > 0)
        object_features.copyTo(featuresDatabase(cv::Rect(0,background_features.rows,object_features.cols, object_features.rows)));

    //Same for object classes
    classesDatabase = std::vector<int>();
    for(unsigned int i = 0; i < backgroundClasses.size(); i++)
        classesDatabase.push_back(backgroundClasses[i]);

    for(unsigned int i = 0; i < selectedClasses.size(); i++)
        classesDatabase.push_back(selectedClasses[i]);

    //Get all distances between selected keypoints in squareform and get all angles between selected keypoints
    squareForm = std::vector<std::vector<float> >();
    angles = std::vector<std::vector<float> >();
    for(unsigned int i = 0; i < object_class_kps.size(); i++)
    {
        std::vector<float> lineSquare;
        std::vector<float> lineAngle;
        for(unsigned int j = 0; j < object_class_kps.size(); j++)
        {
            float dx = object_class_kps[j].pt.x-object_class_kps[i].pt.x;
            float dy = object_class_kps[j].pt.y-object_class_kps[i].pt.y;
            lineSquare.push_back(sqrt(dx*dx+dy*dy));
            lineAngle.push_back(atan2(dy, dx));
        }
        squareForm.push_back(lineSquare);
        angles.push_back(lineAngle);
    }


    //Find the center of selected keypoints
    cv::Point2f center(0,0);
    for(unsigned int i = 0; i < object_class_kps.size(); i++)
        center += object_class_kps[i].pt;
    center *= (1.0/object_class_kps.size());

    //Remember the rectangle coordinates relative to the center
    centerToTopLeft = rect - center;
    centerToTopRight = cv::Point2f(bottomright.x, rect.y) - center;
    centerToBottomRight = bottomright - center;
    centerToBottomLeft = cv::Point2f(rect.x, bottomright.y) - center;

    //Calculate springs of each keypoint
    springs = std::vector<cv::Point2f>();
    for(unsigned int i = 0; i < object_class_kps.size(); i++)
        springs.push_back(object_class_kps[i].pt - center);

    */
}

boost::container::vector< cv::Rect > DTCCL::detectFace(cv::Mat im_gray){
    ///home/eljeilany/dev/cu+cv/opencv-3.0.0/data/haarcascades/haarcascade_frontalface_alt.xml
    ///home/deployAgent/opencv/opencv-3.0.0/data/haarcascades/haarcascade_frontalface_alt.xml
    boost::container::vector< cv::Rect > rect;
    rect.clear();
    if( !this->face_cascade.load( "/home/deployAgent/opencv/opencv-3.0.0/data/haarcascades/haarcascade_frontalface_alt.xml" ) ){
        std::cout << "error loading classifier" << std::endl;
        return rect;
    }

    std::vector<cv::Rect> faces;

    cv::equalizeHist( im_gray, im_gray );

    //-- Detect faces
    face_cascade.detectMultiScale( im_gray, faces, 1.1, 3, 0|cv::CASCADE_SCALE_IMAGE, cv::Size(15, 15) );

    for ( size_t i = 0; i < faces.size(); i++ ){
        std::cout << "one" << std::endl;
        rect.push_back(faces[i]);
    }
    return rect;
}

boost::container::vector< cv::Rect > DTCCL::detectPed(cv::Mat im_gray){

    cv::HOGDescriptor hog;
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
    std::vector<cv::Rect> found;
    boost::container::vector< cv::Rect > rect;
    //std::cout << "ready for d" << std::endl;
    hog.detectMultiScale(im_gray, found, 0, cv::Size(8,8), cv::Size(32,32), 1.05, 2);
    //std::cout << "after d" << std::endl;
    size_t i, j;
        for (i=0; i<found.size(); i++)
        {
            Rect r = found[i];
            for (j=0; j<found.size(); j++)
                if (j!=i && (r & found[j]) == r)
                    break;
            if (j== found.size())
                rect.push_back(r);
        }
    return rect;
}

void DTCCL::learn(std::vector<cv::KeyPoint> Keypoints, cv::Point2f center, float scaleEstimate, float rotationEstimate,
                  cv::Mat features, boost::container::stable_vector<std::pair<cv::KeyPoint, int> > inliers,
                  boost::container::stable_vector<std::pair<cv::KeyPoint,int> > fusedKeypoints,boost::container::stable_vector<std::pair<cv::KeyPoint, int> > consunsusMatched, const cv::Rect rect){
    // some of the Kps are still not detected

    /*boost::container::stable_vector<std::pair<cv::KeyPoint, int> > consunsusMatched;
    ConsensusMatch(Keypoints,features,center,scaleEstimate,rotationEstimate,consunsusMatched);*/

    // Update weights
    for(unsigned int i=0;i<fusedKeypoints.size();i++){
        if(inliermatch[i]){
            if(weight[fusedKeypoints[i].second]<120){
                weight[fusedKeypoints[i].second]+=5;
            }
        }else{
            if(weight[fusedKeypoints[i].second]>-121){
                weight[fusedKeypoints[i].second]-=5;
            }
        }
    }

    for(unsigned int i=0;i<consunsusMatched.size();i++){
        if(weight[consunsusMatched[i].second]<120)
        weight[consunsusMatched[i].second]+=5;
    }

    boost::container::stable_vector<int> newKeypoints;
    newKeypoints.reserve(Keypoints.size());
    for(unsigned int i=0;i<Keypoints.size();i++){
        if(!matchfound[i] && !objectmatch[i] && rect.contains(Keypoints[i].pt)){
            newKeypoints.push_back(i);
        }

        if(matchfound[i] && !objectmatch[i] && rect.contains(Keypoints[i].pt)){
            if(selectedlearn[i]>= 0 && selectedlearn[i]<= weight.size() ){
            if(weight[selectedlearn[i]]<120)
            weight[selectedlearn[i]] += 5;
            }
        }
    }

    // Update Active Kps
    activeKeypoints.clear();

    boost::container::stable_vector<std::pair<cv::KeyPoint, int> > PkP;

    fuse(consunsusMatched,inliers,PkP);

    activeKeypoints.reserve(PkP.size());
    for(unsigned int i=0;i<PkP.size();i++){
        if(weight[PkP[i].second] > 4){
            activeKeypoints.push_back(PkP[i]) ;
        }
    }


    // Add the new kepoints
    if( PkP.size()>= 0.4* nbInitialKeypoints && weight.size() <= 2 * nbInitialKeypoints && scaleEstimate < 1.2){
    boost::container::stable_vector<signed char> weights =weight;
    classesDatabase.reserve(classesDatabase.size() + newKeypoints.size() );
    springs.reserve(springs.size() + newKeypoints.size());
    activeKeypoints.reserve(activeKeypoints.size() + newKeypoints.size());
    weights.reserve( weight.size() + newKeypoints.size());

    for(unsigned int i=0;i<newKeypoints.size();i++){

        int index = newKeypoints[i];
        std::cout<< "class " << (classesDatabase[classesDatabase.size() -1 ] + 1 ) << std::endl ;
        // Add the new class
        classesDatabase.push_back( classesDatabase[classesDatabase.size() -1 ] + 1);

        // Add the new descriptor to the database
        cv::vconcat(featuresDatabase,features.row(index),featuresDatabase);

        // Compute and add the new point spring
        springs.push_back( (1.0/scaleEstimate) * rotate(Keypoints[index].pt - center, -rotationEstimate) );

        // add to active Keypoints
        activeKeypoints.push_back(std::make_pair(Keypoints[index], classesDatabase[classesDatabase.size() -1 ] ));
        weights.push_back(5);
    }

    weight.reserve(weights.size());
    this->weight = weights;
    // Update springs

    //Get all distances between selected keypoints in squareform and get all angles between selected keypoints
    size_t num_points = springs.size();

    //Create matrices of pairwise distances/angles
    squareForm = cv::Mat(num_points, num_points, CV_32FC1);
    angles = cv::Mat(num_points, num_points, CV_32FC1);

    for (size_t i = 0; i < num_points; i++)
    {
        for (size_t j = 0; j < num_points; j++)
        {
            cv::Point2f v = springs[i] - springs[j];

            float distance = cv::norm(v);
            float angle = std::atan2(v.y,v.x);

            squareForm.at<float>(i,j) = distance;
            angles.at<float>(i,j) = angle;
        }

    }
    }
}
signed char DTCCL::weightf(unsigned i){
    return this->weight[i];
}
void DTCCL::match(const std::vector<cv::KeyPoint> & keypoints, const cv::Mat features,
                 boost::container::stable_vector<std::pair<cv::KeyPoint,int> >& matchedKeypoints) {
                   unsigned ws = this->weight.size();
                   std::cout<< std::endl << "weights " << ws << std::endl;
                   std::vector<std::vector<cv::DMatch> > matches;
                   std::vector<unsigned int> db = classesDatabase;
                   //descriptorMatcher->match(featuresDatabase,features, matches);
                   descriptorMatcher->knnMatch(featuresDatabase,features,matches,2);
                   for (unsigned int i = 0; i < matches.size(); i++)
                    {
                       std::vector<cv::DMatch> m = matches[i];


                       float distance1 = m[0].distance ;
                      if(m.size()>= 2){
                          float distance2 = m[1].distance ;
                          if (distance1 > distance2*thrratio) continue;
                      }
                      if(m[0].trainIdx>= db.size()) continue;
                      int matched_class = db[m[0].trainIdx];

                       //if (distance1 > thrdist) continue;

                       matchfoundC(i,true);
                       if (matched_class < 0 ) continue ;
                       if (matched_class == 0) continue;
                       //std::cout<< endl << "class " << matched_class << endl;
                       if(0 < matched_class && matched_class < ws){
                          // std::cout << "   and weight " << +weights[matched_class] <<endl;
                           if (weightf(matched_class) < 0) {
                               selectedlearnC(i, matched_class);
                               continue;
                           }
                           //matchedKeypoints.emplace_back(std::make_pair(keypoints[i],matched_class));
                           matchedKeypoints.push_back(std::make_pair(keypoints[i],matched_class));
                           objectmatchC(i, true);
                       }


                   }
}


// This function Robutstly tracks a set of keypoints from the frame t-1 to t
void DTCCL::track(cv::Mat im_prev, cv::Mat im_gray, const boost::container::stable_vector<std::pair<cv::KeyPoint, int> >& keypointsIN,
                  boost::container::stable_vector<std::pair<cv::KeyPoint, int> >& keypointsTracked, int THR_FB){
    // Status of tracked keypoint - True means successfully tracked
    std::vector<unsigned char> status;
     // = std::vector<unsigned char>();
    //for(int i = 0; i < keypointsIN.size(); i++)
      //  status.push_back(false);
    //If at least one keypoint is active
    if(keypointsIN.size() > 0)
    {
        std::vector<cv::Point2f> pts;
        std::vector<cv::Point2f> pts_back;
        std::vector<cv::Point2f> nextPts;
        std::vector<unsigned char> status_back;
        std::vector<float> err;
        std::vector<float> err_back;
        std::vector<float> fb_err;
        for(unsigned int i = 0; i < keypointsIN.size(); i++)
            pts.push_back(cv::Point2f(keypointsIN[i].first.pt.x,keypointsIN[i].first.pt.y));

        //Calculate forward optical flow for prev_location
        cv::calcOpticalFlowPyrLK(im_prev, im_gray, pts, nextPts, status, err);
        //Calculate backward optical flow for prev_location
        cv::calcOpticalFlowPyrLK(im_gray, im_prev, nextPts, pts_back, status_back, err_back);

        //Calculate forward-backward error
        for(unsigned int i = 0; i < pts.size(); i++)
        {
            cv::Point2f v = pts_back[i]-pts[i];
            fb_err.push_back(sqrt(v.dot(v)));
        }

        //Set status depending on fb_err and lk error
        for(unsigned int i = 0; i < status.size(); i++)
            status[i] = (fb_err[i] <= THR_FB) & status[i];

        keypointsTracked = boost::container::stable_vector<std::pair<cv::KeyPoint, int> >();
        for(unsigned int i = 0; i < pts.size(); i++)
        {
            std::pair<cv::KeyPoint, int> p = keypointsIN[i];
            if(status[i])
            {
                p.first.pt.x = nextPts[i].x;
                p.first.pt.y = nextPts[i].y;
                keypointsTracked.push_back(p);
            }
        }
    }
    else keypointsTracked = boost::container::stable_vector<std::pair<cv::KeyPoint, int> >();
}


//
void DTCCL::estimateScaleRotation(const boost::container::stable_vector<std::pair<cv::KeyPoint, int> >& keypoints,
        float & scale, float & rotation) {
    //Compute pairwise changes in scale/rotation
    std::vector<float> changes_scale;
    changes_scale.reserve(keypoints.size()*keypoints.size());
    std::vector<float> changes_angles;
    changes_angles.reserve(keypoints.size()*keypoints.size());
    // maxS = 0.0;
    // minS = std::numeric_limits<t_float>::infinity();

    for (size_t i = 0; i < keypoints.size(); i++)
    {
        for (size_t j = 0; j < keypoints.size(); j++)
        {
            if (keypoints[i].second != keypoints[j].second)
            {
                cv::Point2f v = keypoints[i].first.pt - keypoints[j].first.pt;

                    float distance = cv::norm(v);
                    float distance_original = squareForm.at<float>(keypoints[i].second,keypoints[j].second);
                    float change_scale = distance / distance_original;
                    changes_scale.push_back(change_scale);


                    float angle = std::atan2(v.y,v.x);
                    float angle_original = angles.at<float>(keypoints[i].second,keypoints[j].second);
                    float change_angle = angle - angle_original;

                    //Fix long way angles
                    if (fabs(change_angle) > M_PI) {
                        change_angle = DTCCL::sgn(change_angle) * 2 * M_PI + change_angle;

                    }


                    changes_angles.push_back(change_angle);


            }

        }

    }

    //Do not use changes_scale, changes_angle after this point as their order is changed by median()
    if (changes_scale.size() < 2) scale = 1;
    else scale = median2(changes_scale);

    if (changes_angles.size() < 2) rotation = 0;
    else rotation = median2(changes_angles);
}

// This fuction fuses two keypoints sets
void DTCCL::fuse(const boost::container::stable_vector< std::pair< cv::KeyPoint,int> >& first,
          const boost::container::stable_vector<std::pair<cv::KeyPoint,int> >& second,
          boost::container::stable_vector<std::pair<cv::KeyPoint,int> >& fused){
    fused.clear();
    for (size_t i = 0; i < first.size(); i++)
    {
    if(first[i].second == 0) continue;
    fused.push_back(first[i]);
    }

    for (size_t i = 0; i < second.size(); i++)
    {
        int class_second = second[i].second ;
        if( class_second == 0) continue;
        bool found = false;
        for (size_t j = 0; j < first.size(); j++)
        {
            int class_first = first[j].second;
            if (class_first == class_second){
            found = true;
            }
        }

        if (!found)
        {
            fused.push_back(second[i]);
        }

    }

}

// Look for consensus in the voptes and estimate the center
void DTCCL::Consensus(const boost::container::stable_vector<std::pair<cv::KeyPoint, int> >& keypoints,
        const float scale, const float rotation, cv::Point2f & center,
        boost::container::stable_vector<std::pair<cv::KeyPoint, int> >& inlier){
    //If no points are available, reteurn nan
    //std::cout << "Consensus called";
    if (keypoints.size() == 0)
    {
        center.x = numeric_limits<float>::quiet_NaN();
        center.y = numeric_limits<float>::quiet_NaN();
        return;
    }

    //Compute votes
    //std::cout << "vote compute";
    std::vector<cv::Point2f> votes(keypoints.size());
    for (size_t i = 0; i < keypoints.size(); i++)
    {
        votes[i] = keypoints[i].first.pt - scale * rotate(springs[keypoints[i].second], rotation);//springs[keypoints[i].second] ;//scale * rotate(springs[keypoints[i].second], rotation);
    }
    //std::cout << "vote computed";
    t_index N = keypoints.size();

    float * D = new float[N*(N-1)/2]; //This is a lot of memory, so we put it on the heap
    cluster_result Z(N-1);
    //std::cout << "data alloked";
    //Compute pairwise distances between votes
    int index = 0;
    for (size_t i = 0; i < keypoints.size(); i++)
    {
        for (size_t j = i+1; j < keypoints.size(); j++)
        {
            //TODO: This index calculation is correct, but is it a good thing?
            //int index = i * (points.size() - 1) - (i*i + i) / 2 + j - 1;
            D[index] = cv::norm(votes[i] - votes[j]);
            if (fc_isnan(D[index])){
            D[index] = std::numeric_limits<t_float>::infinity();
            };
            index++;
        }
    }

    std::cout << "fastcluster::MST_linkage_core() call";
    MST_linkage_core(N,D,Z);
    std::cout << "fastcluster::MST_linkage_core() return";

    union_find nodes(N);

    //Sort linkage by distance ascending
    std::stable_sort(Z[0], Z[N-1]);

    //S are cluster sizes
    int S[2*N-1];
    //TODO: Why does this loop go to 2*N-1? Shouldn't it be simply N? Everything > N gets overwritten later
    for(int i = 0; i < 2*N-1; i++)
    {
        S[i] = 1;
    }

    t_index parent = 0; //After the loop ends, parent contains the index of the last cluster
    for (cnode const * NN=Z[0]; NN!=Z[N-1]; ++NN)
    {
        // Get two data points whose clusters are merged in step i.
        // Find the cluster identifiers for these points.
        t_index node1 = nodes.Find(NN->node1);
        t_index node2 = nodes.Find(NN->node2);

        // Merge the nodes in the union-find data structure by making them
        // children of a new node
        // if the distance is appropriate
        if (NN->dist < thrcutoff)
        {
            parent = nodes.Union(node1, node2);
            S[parent] = S[node1] + S[node2];
        }
    }

    //Get cluster labels
    int T[N];
    for (t_index i = 0; i < N; i++)
    {
        T[i] = nodes.Find(i);
    }

    //Find largest cluster
    int S_max = std::distance(S, std::max_element(S, S + 2*N-1));

    //Find inliers, compute center of votes
    inlier.reserve(S[S_max]);
    center.x = 0;
    center.y = 0;

    for (size_t i = 0; i < keypoints.size(); i++)
    {
        //If point is in consensus cluster
        if (T[i] == S_max)
        {
            inlier.push_back(keypoints[i]);
            inliermatch[i] = true;
            center.x += votes[i].x;
            center.y += votes[i].y;
        }

    }

    center.x /= inlier.size();
    center.y /= inlier.size();

    delete[] D;
}

boost::container::stable_vector<std::pair<cv::KeyPoint, int> > DTCCL::clusterConsensus(const boost::container::stable_vector<std::pair<cv::KeyPoint, int> > &keypointsIN,
  const float scaleEstimate, const float medRot,
                             cv::Point2f &center){
    //std::cout << "start C" << std::endl;
    center = cv::Point2f(NAN,NAN);
    std::vector<PairInt> list;
    boost::container::stable_vector<std::pair<cv::KeyPoint, int> > keypoints;
    for(unsigned int i = 0; i < keypointsIN.size(); i++)
        list.push_back(std::make_pair(keypointsIN[i].second, i));
    std::sort(&list[0], &list[0]+list.size(), comparatorPair<int>);
    for(unsigned int i = 0; i < list.size(); i++)
        keypoints.push_back(keypointsIN[list[i].second]);

    /** Votes **/
    // Initialize the votes container and calculate the votes base on the vector springs.
    //std::cout << "start V"<< std::endl;
    votes = boost::container::stable_vector<cv::Point2f>();
    for(unsigned int i = 0; i < keypoints.size(); i++)
        votes.push_back( scaleEstimate * rotate(springs[keypoints[i].second], medRot)); //keypoints[i].first.pt - springs[keypoints[i].second]) ;//
        //std::cout << "start linkn"<< std::endl;
    // Compute linkage between pairwise distances
    boost::container::stable_vector<Cluster> linkageData = linkage(votes);
    //std::cout << "start clstr"<< std::endl;
    // Perform hierarchical distance-based clustering
    boost::container::stable_vector<int> T = fcluster(linkageData, thrcutoff);

    // Count votes for each cluster
    boost::container::stable_vector<int> cnt = binCount(T);
    //std::cout << "bin c done"<< std::endl;
    // Get largest class
    int Cmax = argmax(cnt);

    // Remember outliers
    outliers = boost::container::stable_vector<std::pair<cv::KeyPoint, int> >();
    boost::container::stable_vector<std::pair<cv::KeyPoint, int> > newKeypoints;
    boost::container::stable_vector<cv::Point2f> newVotes;
    for(unsigned int i = 0; i < keypoints.size(); i++)
    {
        if(T[i] != Cmax)
            outliers.push_back(keypoints[i]);
        else
        {
            newKeypoints.push_back(keypoints[i]);
            newVotes.push_back(votes[i]);
        }
    }
    keypoints = newKeypoints;
    //std::cout << "C prep"<< std::endl;
    center = cv::Point2f(0,0);
    for(unsigned int i = 0; i < newVotes.size(); i++)
        center += newVotes[i];
    center *= (1.0/newVotes.size());
    //std::cout << "cluster ret" << std::endl;
    return keypoints;

}

void DTCCL::ConsensusMatch(const std::vector<cv::KeyPoint> & keypoints, const cv::Mat features,
                const cv::Point2f center, const float scale, const float rotation,
                boost::container::stable_vector<std::pair<cv::KeyPoint,int> >& matchedKeypoints){
    if (keypoints.size() == 0) {
        std::cout << "Matcher::matchLocal() return";
        return;
    }

    //Transform initial points
    std::vector<unsigned int> db = classesDatabase;
    boost::container::vector<cv::Point2f> pts_fg_trans;
    pts_fg_trans.reserve(springs.size());
    //std::cout << "ready for Cs" << std::endl;
    for (unsigned int i = 0; i < springs.size(); i++)
    {
        pts_fg_trans.push_back(scale * rotate(springs[i], -rotation));
    }
    //Perform local matching
    //std::cout << "ready for Cs good" << std::endl;
    for (size_t i = 0; i < keypoints.size(); i++)
    {
        //Normalize keypoint with respect to center
        cv::Point2f location_rel = keypoints[i].pt - center;

        //Find potential indices for matching
        std::vector<unsigned int> indices_potential;
        for (size_t j = 0; j < pts_fg_trans.size(); j++)
        {
            float l2norm = cv::norm(pts_fg_trans[j] - location_rel);

            if (l2norm < thrcutoff) {
                indices_potential.push_back( backgroundClasses.size() + j);
            }

        }

        //If there are no potential matches, continue
        if (indices_potential.size() == 0) continue;

        //Build descriptor matrix and classes from potential indices
        cv::Mat database_potential = Mat(indices_potential.size(), featuresDatabase.cols, featuresDatabase.type());
        for (size_t j = 0; j < indices_potential.size(); j++) {
            featuresDatabase.row(indices_potential[j]).copyTo(database_potential.row(j));
        }

        //Find distances between descriptors
        std::vector<std::vector<cv::DMatch> > matches;
        //descriptorMatcher->match(features.row(i),database_potential, matches,2);
        descriptorMatcher->knnMatch(features.row(i),database_potential, matches,2);
        std::vector<cv::DMatch> m = matches[0];

        float distance1 = m[0].distance ;
        //if (distance1 > thrdist) continue;
        if(m.size()>= 2){
        float distance2 = m[1].distance ;
        if (distance1 > distance2*thrratio) continue;
        }

        unsigned int matched_class = db[indices_potential[(unsigned int)m[0].trainIdx]];
        //std::cout << "class " << matched_class  << "   and weight " << + DTCCL::weight[matched_class] <<std::endl;;
        //std::cout << "accessed" << std::endl;
        matchfoundC(i, true);
        objectmatchC(i, true);
        matchedKeypoints.push_back(std::make_pair(keypoints[i],matched_class));
    }
}

void DTCCL::processFrame2(cv::Mat im_gray) {
    std::vector<cv::KeyPoint> keypoints;

    //detector->detect(im_gray, keypoints);
    //std::vector<std::pair<cv::KeyPoint,int> > kp;
    //keypoints = activeKeypoints;
/*
    for (size_t i = 0; i < keypoints.size(); i++){
            kp.push_back( std::make_pair(keypoints[i],i) );
    }*/


    DTCCL::activeKeypoints = DTCCL::trackedKeypoints;
    //activeKeypoints.clear();
    //activeKeypoints = trackedKeypoints;
    //DTCCL::trackedKeypoints.clear();
    //activeKeypoints = kp;
    // Track object class Keypoints
    //trackedKeypoints = std::vector<std::pair<cv::KeyPoint, int> >();
    track( im_prev, im_gray, DTCCL::activeKeypoints, DTCCL::trackedKeypoints);
    DTCCL::prvkp = DTCCL::activeKeypoints;

    DTCCL::im_prev = im_gray.clone();

}

// Processing the next frame
void DTCCL::processFrame(cv::Mat im_gray) {
std::cout << "tracking... phase1..." << std::endl;

    // Track object class Keypoints
    trackedKeypoints = boost::container::stable_vector<std::pair<cv::KeyPoint, int> >();
    track(im_prev, im_gray, activeKeypoints, trackedKeypoints);
    /*
    std::cout << "tracking... trackedKeypoints..." << std::endl;
    for(int i=0; i < trackedKeypoints.size(); i++){
    std::cout << "class " << trackedKeypoints[i].second << " weight " << +weight[trackedKeypoints[i].second] << std::endl;
    }*/

    std::cout << "tracked... " << trackedKeypoints.size() << " phase2..." << std::endl;


    //Detect keypoints, compute descriptors
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat features;
    detector->detect(im_gray, keypoints);
    descriptorExtractor->compute(im_gray, keypoints, features);


    //std::cout << "tracking... phase2..." << std::endl;

    // match
    matchfound.clear();
    objectmatch.clear();
    selectedlearn.clear();

    matchfound = std::vector<bool>(keypoints.size(),false);

    objectmatch = std::vector<bool>(keypoints.size(),false);

    selectedlearn = std::vector<unsigned int>(keypoints.size(),-1);

    boost::container::stable_vector<std::pair<cv::KeyPoint,int> > matchedKeypoints;
    std::cout << "tracking... match..." << std::endl;
    match(keypoints,features,matchedKeypoints);
    std::cout<< "matched: "<< matchedKeypoints.size() << std::endl << "tracking... fuse..." << std::endl;
    // fuse
    boost::container::stable_vector<std::pair<cv::KeyPoint,int> > fusedKeypoints;
    this->fuse(trackedKeypoints,matchedKeypoints,fusedKeypoints);
    std::cout << "tracking... fused...:" <<fusedKeypoints.size() << std::endl;
    /*for(int i=0; i < fusedKeypoints.size(); i++){
    std::cout << "class " << fusedKeypoints[i].second << " weight " << +weight[fusedKeypoints[i].second] << std::endl;
    }*/
    // Estimate parametres
    if (fusedKeypoints.size()<0.1*nbInitialKeypoints){
      found = false;
      return;
    }
    cv::Point2f center;
    float scaleEstimate;
    float rotationEstimate;
    //std::cout << "tracking... estimate..." << std::endl;
    // Estimate the scale and rotation
    estimateScaleRotation(fusedKeypoints,scaleEstimate,rotationEstimate);

    // Tracked object features
    std::cout << "tracking... consensus..." << std::endl;
    inliermatch.clear();
    inliermatch = boost::container::stable_vector<bool>(fusedKeypoints.size(),false);
    //std::cout << "tracking... inm clear..." << std::endl;
    boost::container::stable_vector<std::pair<cv::KeyPoint, int> > inliers;
    //std::cout << "tracking... inm aloc..." << std::endl;
    //inliers = clusterConsensus(fusedKeypoints,scaleEstimate,rotationEstimate,center);

    Consensus(fusedKeypoints,scaleEstimate,rotationEstimate,center,inliers);

    boost::container::stable_vector<std::pair<cv::KeyPoint, int> > consunsusMatched;
    //std::cout << "c match call" << std::endl;
    ConsensusMatch(keypoints,features,center,scaleEstimate,rotationEstimate,consunsusMatched);
    //std::cout << "c match ret" << std::endl;
/*
    std::cout << "tracking... detected..." << std::endl;
    for(int i=0; i < inliers.size(); i++){
    std::cout << "class " << inliers[i].second << " weight " << +weight[inliers[i].second] << std::endl;
    }*/
    std::cout << "consensus matched: "<< consunsusMatched.size()<< std::endl;
/*
    for(int i=0; i < consunsusMatched.size(); i++){
    std::cout << "class " << consunsusMatched[i].second << " weight " << +weight[consunsusMatched[i].second] << std::endl;
    }*/

    float deM = 1000.0;
    boost::container::vector< cv::Rect > det = detectFace(im_gray);
    cv::Rect rectd;
    for(unsigned i = 0;i<det.size();i++){
    float rectx = det[i].x + det[i].width/2;
    float recty = det[i].y + det[i].height/2;
    float pd = std::sqrt( std::pow(rectx - center.x,2) + std::pow(recty - center.y, 2 ) );
        if( pd < deM){
            deM = pd;
            rectd = det[i];
        }
    }
    std::cout << "det ok "<< deM << "pixl" << std::endl;
    int x = inliers.size() ;
    im_prev = im_gray.clone() ;
    // it is what is
    if(  (x >= 0.15*nbInitialKeypoints && scaleEstimate<1.8) || (matchedKeypoints.size() > 0.1*nbInitialKeypoints && deM<80 && det.size() > 0)|| (consunsusMatched.size() > 0.1*nbInitialKeypoints && deM<80 && det.size() > 0)
          || (consunsusMatched.size() > 0.1*nbInitialKeypoints )){
    found = true;
    drawKeypoints = trackedKeypoints;
    if(consunsusMatched.size()< 0.1 && det.size() > 0){
      center.x = rectd.x + rectd.width/2;
      center.y = rectd.x + rectd.height/2;
      scaleEstimate = rectd.width/insize.width;
    }
    cv::RotatedRect r = cv::RotatedRect(center,insize * scaleEstimate, rotationEstimate/CV_PI * 180);
    boundingbox = r.boundingRect();
    r.points(vertices);
    activeKeypoints = inliers;
    std::cout << std::endl << "///// CENTER " << center.x <<" "<< center.y << std::endl;
    std::cout << "tracking... learn..." << std::endl;
    drawc = center;
    //fuse(inliers,consunsusMatched,activeKeypoints);
    learn(keypoints,center,scaleEstimate,rotationEstimate,features,inliers,fusedKeypoints,consunsusMatched,boundingbox);
    }else{
    found = false;
    activeKeypoints.clear();
    }

}
