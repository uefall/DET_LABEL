#include <fstream> 
#include <iostream> 
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <vector>
#include <map>
#include <list>
#include <io.h>
#include <direct.h>

using namespace std;
using namespace cv;

#define PRE_LABEL_DIR "new_output.txt"
Point P1(0, 0);
Point P2(0, 0);
bool clicked = false;
bool rect_valid = false;
Rect cropRect(0, 0, 0, 0);
const char* winName = "FULL_image";
string	img_path;


bool hasEnding(std::string const &fullString, std::string const &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}

void dir(string path, list<string> &img_list)
{
	long hFile = 0;
	struct _finddata_t fileInfo;
	string pathName, exdName;

	if ((hFile = _findfirst(pathName.assign(path).append("\\*").c_str(), &fileInfo)) == -1)
	{
		return;
	}

	do {
		if ((strcmp(fileInfo.name, ".") != 0) && (strcmp(fileInfo.name, "..") != 0))
		{
			//cout << fileInfo.name << (fileInfo.attrib&_A_SUBDIR ? "[folder]" : "[file]") << endl;
			if (fileInfo.attrib == _A_SUBDIR)
			{
				string newPath = path + "\\" + fileInfo.name;

				dir(newPath, img_list);
			}
			else
			{
				if (hasEnding(fileInfo.name, ".jpg"))
				{

					//cout << fileInfo.name << " JPG" << endl;
					img_list.push_back(path + "\\" + fileInfo.name);
				}
			}
		}
	} while (_findnext(hFile, &fileInfo) == 0);

	cout << (img_list.size()) << endl;

	_findclose(hFile);
	return;
}


void SplitString(const string& s, vector<string>& v, const string& c)
{
	string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}



void onMouse(int event, int x, int y, int f, void*)
{
	switch (event)
	{
	case  EVENT_LBUTTONDOWN:
		clicked = true;

		P1.x = x;
		P1.y = y;
		P2.x = x;
		P2.y = y;
		break;

	case  EVENT_LBUTTONUP:
		P2.x = x;
		P2.y = y;
		clicked = false;
		rect_valid = true;
		//printf("valid:%d\n", rect_valid);
		break;

	case  EVENT_MOUSEMOVE:
		if (clicked) {
			P2.x = x;
			P2.y = y;
		}
		break;

	default:break;
	}

	if (clicked)
	{
		if (P1.x > P2.x) {
			cropRect.x = P2.x;
			cropRect.width = P1.x - P2.x;
		}
		else {
			cropRect.x = P1.x;
			cropRect.width = P2.x - P1.x;
		}

		if (P1.y > P2.y) {
			cropRect.y = P2.y;
			cropRect.height = P1.y - P2.y;
		}
		else {
			cropRect.y = P1.y;
			cropRect.height = P2.y - P1.y;
		}
	}
}
int checkBoundary(Mat img,Rect &rect) 
{
	//check croping rectangle exceed image boundary

	if (rect.width>img.cols - rect.x)
		rect.width = img.cols - rect.x;
	if (rect.height>img.rows - rect.y)
		rect.height = img.rows - rect.y;
	if (rect.x < 0)
	{
		rect.width += rect.x;
		rect.x = 0;
	}
	if ((rect.x+rect.width)>img.cols)
	{
		rect.width = img.cols - rect.x;
	}
		
	if (rect.y < 0)
	{
		rect.height += rect.y;
		rect.y = 0;
	}
	if ((rect.y + rect.height) > img.rows)
	{
		rect.height = img.rows - rect.y;
	}

	if (rect.height<=0 || rect.width<=0)
	{
		return 0;
	}
	return 1;
}

void showImage(Mat src, vector<Rect> info)
{
	Mat img;
	img = src.clone();
	checkBoundary(img,cropRect);

	//if (cropRect.width>0 && cropRect.height>0) 
	//{
	//	Mat ROI = src(cropRect);
	//	//cv::namedWindow("cropped", WINDOW_AUTOSIZE);
	//	imshow("cropped", ROI);

	//}
	//else if (info.size() != 0)
	//{
	//	Rect real_rect = info[0];
	//	Mat ROI = src(real_rect);
	//	//cv::namedWindow("cropped", WINDOW_AUTOSIZE);
	//	imshow("cropped", ROI);

	//}

	rectangle(img, cropRect, Scalar(0, 255, 0), 1, 8, 0);

	for (size_t i = 0; i < info.size(); i++) 
	{
		Rect d = info[i];
		checkBoundary(img, d);
		rectangle(img, d, Scalar(255, 0, 0), 1, 8, 0);

	}

	imshow(winName, img);
	//createTrackbar("bar",winName,)
}

int info2txt(map<string, vector<Rect>>	obj_info)
{
	FILE	*fp;
	string save_path;
	save_path = img_path + "\\result.txt";
	fp = fopen(save_path.c_str(), "wb");

	map<string, vector<Rect>>::iterator iter;//定义一个迭代指针iter
	for (iter = obj_info.begin(); iter != obj_info.end(); iter++)
	{
		string img_name = iter->first;
		vector<Rect> info = iter->second;
		size_t pos = img_name.find(img_path);
		if (pos != std::string::npos)
		{
			img_name.erase(pos, img_path.length());
		}
		fprintf(fp, "%s ", img_name.c_str());
		for (size_t i = 0; i < info.size(); i++) {
			Rect d = info[i];
			fprintf(fp, "1 %d %d %d %d\n", d.x, d.y, d.width, d.height);
		}
	}

	fclose(fp);
	printf("info\n");
	return 1;
}

int crop_img(map<string, vector<Rect>>	obj_info)
{
	FILE	*fp;
	string save_path, id_path;
	save_path = img_path + "\\crop_result\\";
	if(_access(save_path.c_str(), 0) == -1)
	{
		_mkdir(save_path.c_str());
	}


	map<string, vector<Rect>>::iterator iter;//定义一个迭代指针iter
	for (iter = obj_info.begin(); iter != obj_info.end(); iter++)
	{
		string img_name = iter->first;
		vector<Rect> info = iter->second;
		Mat org_img = imread(img_name);
		string split_c = "\\";
		string ext_c = ".jpg";
		vector<string> str_list;
		size_t pos = img_name.find(ext_c);
		if (pos != std::string::npos)
		{
			img_name.erase(pos, img_path.length());
		}
		SplitString(img_name, str_list, split_c);
		string name_only = str_list.back();
		str_list.pop_back();
		string id_label = str_list.back();

		id_path = save_path + id_label + "\\";
		if (_access(id_path.c_str(), 0) == -1)
		{
			_mkdir(id_path.c_str());
		}

		for (size_t i = 0; i < info.size(); i++) {
			Rect d = info[i];
			int re = checkBoundary(org_img, d);
			if (re == 0)
			{
				//obj_info.erase(iter);
				break;
			}
			Mat sub_img = org_img(d);
			string crop_path = id_path + name_only + "_crop.jpg";
			imwrite(crop_path, sub_img);

		}
	}

	printf("crop fin\n");
	return 1;
}

int txt2info(map<string, vector<Rect>>	&obj_info)
{
	FILE	*fp;
	string save_path;
	string img_path_f;
	char	img_path_r[1024];

	save_path = img_path + "\\result.txt";
	fp = fopen(save_path.c_str(), "r");
	if (fp == NULL)
	{
		printf("无已存在的标定，重新开始\n");
		return 0;
	}
	Rect rect_r;
	while (5 == fscanf(fp, "%s 1 %d %d %d %d", img_path_r, &rect_r.x, &rect_r.y, &rect_r.width, &rect_r.height))
	{
		vector<Rect> rect_v;
		rect_v.push_back(rect_r);

		img_path_f = img_path + img_path_r;
		obj_info[img_path_f] = rect_v;
	}
	
	fclose(fp);
	printf("获取已存在的标定\n");
	return 1;
}

int main(int argc,char* argv[])
{
	
	list<string>				img_list;
	string						temp;
	map<string, vector<Rect>>	obj_info;
	bool						crop_flg = 0;

	if (argc < 2) {
		printf("请输入要处理的图片路径\n");
		return -1;
	}
	if (argc == 3)
	{
		crop_flg = atoi(argv[2]);
	}

	img_path = string(argv[1]);
	dir(img_path, img_list);
	list<string> ::iterator cur_img = img_list.begin();
	cout << _pgmptr << endl;

	txt2info(obj_info);

	if (img_list.size()==0)
	{
		printf("目录内无图片，请检查路径参数。\n");
		return 0;
	}

	while (1)
	{

		char c = waitKey(40);
		if (c == 'c'|| c == 27) break;

		if (c == 'a')
		{
			if (cur_img != img_list.begin())
			{
				if (rect_valid)
				{
					vector<Rect> rect_v;
					rect_v.push_back(cropRect);
					obj_info[*cur_img] = rect_v;
					cropRect = Rect(0, 0, 0, 0);
					rect_valid = false;
				}
				cur_img--;
				cout << *cur_img << endl;
			}
			else
				printf("已达到第一张\n");
		}
		if (c == 'd' || c == ' ')
		{
			if (cur_img != img_list.end())
			{
				if (rect_valid)
				{
					vector<Rect> rect_v;
					rect_v.push_back(cropRect);
					obj_info[*cur_img] = rect_v;
					cropRect = Rect(0, 0, 0, 0);
					rect_valid = false;
				}

				cur_img++;
				if (cur_img == img_list.end())
				{
					printf("已达到最后一张\n");
					cur_img--;
				}
				cout << *cur_img << endl;

			}
			else
				printf("已达到最后一张\n");

		}


		Mat img = imread(*cur_img);
		cv::namedWindow(winName, WINDOW_AUTOSIZE);
		cv::setMouseCallback(winName, onMouse, NULL);
		
		map<string, vector<Rect>>::iterator it = obj_info.find(*cur_img);
		if (it != obj_info.end())
		{
			vector<Rect>	cur_info;
			cur_info = obj_info[*cur_img];
			showImage(img, cur_info);

		}
		else
		{
			vector<Rect> cur_info;
			showImage(img, cur_info);
		}
	}


	info2txt(obj_info);

	if (crop_flg)
	{
		crop_img(obj_info);
	}

	return 0;

}