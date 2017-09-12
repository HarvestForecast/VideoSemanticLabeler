#ifndef VIDEOCONTROL_H
#define VIDEOCONTROL_H
#include <QString>
#include <opencv.hpp>
#include <QVector>
#include <QReadWriteLock>


class VideoControl
{
public:
    VideoControl();
	virtual ~VideoControl();
public:
    bool open(QString filePath);
    bool isOpenned();
    void release();
    QVector<QString> getAllInfos();
    void retrievePalyInfos();
    bool getFrame(cv::Mat& img);
    bool getFrame(cv::Mat& img,double frameNum);
	void forwardFrames(int n);
    void reset(int frameNumber=0);
    double getWidth();
    double getHeight();
    double getFps();
    double getFrameCount();
    double getFourcc();
    double getFormat();
    double getPosMsec();
    double getPosFrames();
    double getPosAviRatio();
    double getMode();
	unsigned int getSkipFrameNum();
	void setSkipFrameNum(unsigned int num = 1);
	void setPosFrames(int idx);
public:
    struct PROPS
    {
        double _width;
        double _height;
        double _fps;
        double _frame_count;
        double _fourcc;
        double _format;
        double _pos_msec;
        double _pos_frames;
        double _pos_avi_ratio;
        double _mode;
    }props;
private:
    cv::VideoCapture _videoCap;

	unsigned int _skipFrameNum;
    QString _filePath;
    mutable QReadWriteLock _lock;
};

#endif // VIDEOCONTROL_H
