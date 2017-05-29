#include <opencv2/core/core.hpp>
#include <opencv2/ml/ml.hpp>

using namespace cv;

void trainSVM(const Mat& trainingData, const Mat& labels, const Mat& inputIm, CvSVM& SVM)
{
    // Set up SVM's parameters
    CvSVMParams params;
    params.svm_type    = CvSVM::C_SVC;
    params.kernel_type = CvSVM::LINEAR;
    params.term_crit   = cvTermCriteria(CV_TERMCRIT_ITER, 1000, 1e-6);	// possible later changes to max number of iterations and stop criterium, for now it works

    // Train the SVM based on the features
    SVM.train(trainingData, labels, Mat(), Mat(), params);
}
