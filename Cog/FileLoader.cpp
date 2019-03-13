#include "pch.h"
#include "FileLoader.h"

FileLoader::FileLoader()
{
	myThreadPool = new ThreadPool(4);
}

FileLoader::~FileLoader()
{
	delete myThreadPool;
	myThreadPool = nullptr;
}

void FileLoader::LoadFile(const String& aPath, const ObjectFunctionView<void(ArrayView<u8>)>& aCallback)
{
	myThreadPool->QueueSingle([this, aPath, aCallback]()
	{
		this->DoLoadWork(aPath, aCallback);
	});
}

void FileLoader::Tick()
{
	for (const FinishedFile& file : myFinishedFiles.Gather())
	{
		if (file.callback.IsValid())
			file.callback.Call(file.data);
	}
}

void FileLoader::DoLoadWork(const String& aPath, const ObjectFunctionView<void(ArrayView<u8>)>& aCallback)
{
	FinishedFile file;
	file.callback = aCallback;

	std::ifstream stream(aPath.GetData(), std::ios::binary | std::ios::end);
	const i32 size = CastBoundsChecked<i32>(stream.tellg());
	stream.seekg(0, stream.beg);
	file.data.Resize(size);

	stream.read(reinterpret_cast<char*>(file.data.GetData()), size);

	myFinishedFiles.Submit(Move(file));
}
