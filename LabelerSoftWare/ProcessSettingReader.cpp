#include "ProcessSettingReader.h"
#include <QMessageBox>
#include <QString>
#include <QDebug>
#include <exception>
using cv::FileNode;
using cv::FileNodeIterator;
ProcessSettingReader::ProcessSettingReader(string filePath):FileStorage(filePath,FileStorage::READ)
{
	if (!this->isOpened())
	{
		QMessageBox::information(NULL,"Hint",QString("Cannot open following file\n")+QString(filePath.c_str()),QMessageBox::StandardButton::Cancel);
	}
}


ProcessSettingReader::~ProcessSettingReader()
{
}

bool ProcessSettingReader::parse()
{
	(*this)["InputFilePath"] >> _data.filePath;
	if (_data.filePath.empty()) throw std::exception("Please specify <InputFilePath> tag");
	qDebug() << "InputFilePath:" << _data.filePath.c_str();

	(*this)["OutputDir"] >> _data.outputDir;
	if (_data.outputDir.empty()) throw std::exception("Please specify <OutputDir> tag");
	qDebug() << "OutputDir:" << _data.outputDir.c_str();

	(*this)["Labeling_Frame_Interval"] >> _data.skipFrameNum;
	if (_data.skipFrameNum<=0) throw std::exception("Please specify <Labeling_Frame_Interval> tag as positive integer");
	qDebug() << "OutputDir:" << _data.skipFrameNum;

	FileNode n = (*this)["LabelList"];
	qDebug()<<"LabelList Size:" << n.size();
	if(n.size()==0) throw std::exception("Please specify <LabelList> tag correctly");

	FileNodeIterator it = n.begin(), it_end = n.end();
	for (; it != it_end; ++it)
	{
		Label label;
		(*it)["Name"] >> std::get<0>(label);
		(*it)["R"] >> std::get<1>(label);
		(*it)["G"] >> std::get<2>(label);
		(*it)["B"] >> std::get<3>(label);
		qDebug() << std::get<0>(label).c_str() << "\t" << std::get<1>(label) << "\t"\
			<< std::get<2>(label) << "\t" << std::get<3>(label);
		_data.labelList.push_back(label);
	}
	
	return true;
}

MetaData ProcessSettingReader::getMetaData()
{
	return _data;
}