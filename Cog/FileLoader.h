#pragma once

class FileLoader : public Object
{
public:
	FileLoader();
	~FileLoader();

	void LoadFile(const String& aPath, const ObjectFunctionView<void(ArrayView<u8>)>& aCallback);
	void Tick();

private:
	struct FinishedFile
	{
		Array<u8> data;
		ObjectFunctionView<void(ArrayView<u8>)> callback;
	};
	
	void DoLoadWork(const String& aPath, const ObjectFunctionView<void(ArrayView<u8>)>& aCallback);
	
	ThreadPool* myThreadPool;
	EventList<FinishedFile> myFinishedFiles;
};
