#include "LandmarkTracking.h"


#ifdef CV_CXX11
#include <mutex>
#include <thread>
#include <queue>
#endif


#ifdef CV_CXX11
template <typename T>
class QueueFPS : public std::queue<T>
{
public:
	QueueFPS() : counter(0) {}

	void push(const T& entry)
	{
		std::lock_guard<std::mutex> lock(mutex);

		std::queue<T>::push(entry);
		counter += 1;
		if (counter == 1)
		{
			// Start counting from a second frame (warmup).
			tm.reset();
			tm.start();
		}
	}

	T get()
	{
		std::lock_guard<std::mutex> lock(mutex);
		T entry = this->front();
		this->pop();
		return entry;
	}

	float getFPS()
	{
		tm.stop();
		double fps = counter / tm.getTimeSec();
		tm.start();
		return static_cast<float>(fps);
	}

	void clear()
	{
		std::lock_guard<std::mutex> lock(mutex);
		while (!this->empty())
			this->pop();
	}

	unsigned int counter;

private:
	cv::TickMeter tm;
	std::mutex mutex;
};

template <typename T>
class Queue : public std::queue<T>
{
public:
	Queue(){}

	void push(const T& entry)
	{
		std::lock_guard<std::mutex> lock(mutex);
		std::queue<T>::push(entry);
		
	}

	T get()
	{
		std::lock_guard<std::mutex> lock(mutex);
		T entry = this->front();
		this->pop();
		return entry;
	}

	void clear()
	{
		std::lock_guard<std::mutex> lock(mutex);
		while (!this->empty())
			this->pop();
	}

private:

	std::mutex mutex;
};
#endif  // CV_CXX11

int main()
{

	string model_path = "./models";
	FaceTracking faceTrack(model_path);
	cv::Mat frame;
	cv::VideoCapture cap(0);
	if (!cap.isOpened())
	{
		return -1;
	}

	int frameIndex = 0;
	vector<int> IDs;
	vector<cv::Scalar> Colors;
	cv::Scalar color;
	srand((unsigned int)time(0));//��ʼ������Ϊ���ֵ
	Queue<cv::Mat> framesQueue;
	std::vector<Face> faces;

#ifdef CV_CXX11
	bool process = true;

	Queue<std::vector<Face> > predictionsQueue;
	std::thread processingThread([&]() {
		//std::queue<AsyncArray> futureOutputs;
		cv::Mat blob;
		for (;process;)
		{
			// Get a next frame
			cv::Mat frame;
			{
				if (!framesQueue.empty())
				{
					frame = framesQueue.get();
					framesQueue.clear();  // Skip the rest of frames
				}
			}

			// Process the frame
			if (!frame.empty())
			{
				double t1 = (double)cv::getTickCount();
				if (frameIndex == 0)
				{
					faceTrack.Init(frame);
					frameIndex = 1;
				}
				else {
					faceTrack.update(frame);
				}
				printf("total %gms\n", ((double)cv::getTickCount() - t1) * 1000 / cv::getTickFrequency());
				printf("------------------\n");
				//Sleep(200);
				predictionsQueue.push(faceTrack.trackingFace);
			}

			
		}
	});
	for (;;) {
		
		if (!cap.read(frame))
		{
			break;
		}

		//cv::transpose(frame, frame);
		//cv::flip(frame, frame, -1);
		//cv::flip(frame, frame, 1);

		framesQueue.push(frame.clone());

		if (!predictionsQueue.empty())
		{
			
			faces.clear();
			faces = predictionsQueue.get();

			

			for (int i = 0; i < faces.size(); i++)
			{
				const Face &info = faces[i];
				cv::Rect rect;
				rect.x = info.faceBbox.x1;
				rect.y = info.faceBbox.y1;
				rect.width  = info.faceBbox.x2 - info.faceBbox.x1;
				rect.height = info.faceBbox.y2 - info.faceBbox.y1;
				
				bool isExist = false;
				for (int j = 0; j < IDs.size(); j++)
				{
					if (IDs[j] == info.face_id)
					{
						color = Colors[j];
						isExist = true;
						break;
					}
				}

				if (!isExist)
				{
					IDs.push_back(info.face_id);
					int r = rand() % 255 + 1;
					int g = rand() % 255 + 1;
					int b = rand() % 255 + 1;
					color = cv::Scalar(r, g, b);
					Colors.push_back(color);
				}

				cv::rectangle(frame, rect, color, 2);
				cv::putText(frame,to_string(info.face_id),Point(rect.x,rect.y-20),FONT_HERSHEY_SIMPLEX,2,Scalar(0,0,255),1,8,false);
				for (int j = 0; j < 5; j++)
				{
					cv::Point p = cv::Point(info.faceBbox.ppoint[j], info.faceBbox.ppoint[j + 5]);
					cv::circle(frame, p, 2, color,2);
				}
			}
			
		}

		
		imshow("frame", frame);

		int q = cv::waitKey(30);
		if (q == 27) break;
	}

	process = false;
	processingThread.join();

#else  // CV_CXX11
	for (;;) {
		if (!cap.read(frame))
		{
			break;
		}
		int q = cv::waitKey(1);
		if (q == 27) break;


		//cv::transpose(frame, frame);
		//cv::flip(frame, frame, -1);
		//cv::flip(frame, frame, 1);
		double t1 = (double)cv::getTickCount();

		if (frameIndex == 0)
		{
			faceTrack.Init(frame);
			frameIndex = 1;
		}
		else {
			faceTrack.update(frame);
		}
		printf("total %gms\n", ((double)cv::getTickCount() - t1) * 1000 / cv::getTickFrequency());
		printf("------------------\n");
		
		std::vector<Face> faceActions = faceTrack.trackingFace;
		for (int i = 0; i < faceActions.size(); i++)
		{
			const Face &info = faceActions[i];
			cv::Rect rect;
			rect.x = info.faceBbox.x1;
			rect.y = info.faceBbox.y1;
			rect.width = info.faceBbox.x2 - info.faceBbox.x1;
			rect.height = info.faceBbox.y2 - info.faceBbox.y1;

			bool isExist = false;
			for (int j = 0; j < IDs.size(); j++)
			{
				if (IDs[j] == info.face_id)
				{
					color = Colors[j];
					isExist = true;
					break;
				}
			}

			if (!isExist)
			{
				IDs.push_back(info.face_id);
				int r = rand() % 255 + 1;
				int g = rand() % 255 + 1;
				int b = rand() % 255 + 1;
				color = cv::Scalar(r, g, b);
				Colors.push_back(color);
			}

			rectangle(frame, rect, color, 2);
			for (int j = 0; j < 5; j++)
			{
				cv::Point p = cv::Point(info.faceBbox.ppoint[j], info.faceBbox.ppoint[j + 5]);
				cv::circle(frame, p, 2, color, 2);
			}
		}
		imshow("frame", frame);
	}
#endif  // CV_CXX11
	IDs.clear();
	Colors.clear();
	cap.release();
	cv::destroyAllWindows();
	return 0;
}