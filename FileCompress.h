#pragma once

#include<iostream>
#include<stdio.h>
#include<string>
#include "Heap.h"
#include<assert.h>


using namespace std;
template<class T>
struct HaffManTreeNode
{
	HaffManTreeNode<T>* _left;
	HaffManTreeNode<T>* _right;
	T _weight;

	HaffManTreeNode(const T& w)
		:_left(NULL)
		, _right(NULL)
		, _weight(w)
	{
	}

};
struct CharInfo
{
	typedef unsigned long LongType;

	unsigned char _ch;
	LongType _count;
	string _code;

	CharInfo(LongType count = 0)
		:_count(count)
	{}

	CharInfo operator +(const CharInfo right) const
	{
		return CharInfo(_count + right._count);
	}

	bool operator !=(const CharInfo right) const
	{
		return _count != right._count;
	}
	bool operator >(const CharInfo right) const
	{
		return _count > right._count;
	}
	bool operator <(const CharInfo right) const
	{
		return _count < right._count;
	}

};
template<class T>
class HaffManTree
{

	typedef HaffManTreeNode<T> Node;
public:
	HaffManTree(const T* a, size_t n, const T& invaid)
	{
		struct NodeCompare
		{
			bool operator()(Node* l, Node* r)
			{
				return l->_weight < r->_weight;
			}
		};

		Heap<Node*, NodeCompare> minHeap;

		for (size_t i = 0; i < n; i++)
		{
			if (a[i] != invaid)
			{
				minHeap.Push(new Node(a[i]));
			}
		}
		while (minHeap.Size()>1)
		{
			Node* left = minHeap.Top();
			minHeap.Pop();
			Node* right = minHeap.Top();
			minHeap.Pop();
			Node* parent = new Node(left->_weight + right->_weight);
			parent->_left = left;
			parent->_right = right;
			minHeap.Push(parent);
		}
		_root = minHeap.Top();
	}

	Node *GetRoot()
	{
		return _root;
	}

protected:
	Node* _root;
};


class FileCompress
{
public:
	FileCompress()
	{
		for (int i = 0; i < 256; i++)
		{
			_infos[i]._ch = i;
			_infos[i]._count = 0;
		}
	}
	void Compress(string filename)
	{
		FILE *fout = fopen(filename.c_str(), "rb");
		assert(fout);
		char ch = fgetc(fout);//读取文件字符
		while (ch != EOF)
		{
			_infos[(unsigned char)ch]._count++;
			ch = fgetc(fout);
		}
		CharInfo invalid;
		string code;
		HaffManTree<CharInfo> tree(_infos, 256, invalid);//建立哈夫曼树
		GenerateHaffManCode(tree.GetRoot(), code);//创建哈夫曼编码

		//压缩
		string compressName = filename + ".haffman";
		FILE* fin = fopen(compressName.c_str(), "wb");
		fseek(fout, 0, SEEK_SET);
		ch = fgetc(fout);
		char value = 0;
		int size = 0;
		while (ch != EOF)
		{
			string code = _infos[(unsigned char)ch]._code;
			for (int i = 0; i < code.size(); i++)
			{
				if (code[i] == '1')
				{
					value |= 1;
				}
				++size;
				if (size == 8)
				{
					fputc(value, fin);
					value = 0;
					size = 0;
				}
				value <<= 1;
			}

			ch = fgetc(fout);
		}
		if (size > 0)
		{
			value <<= 7 - size;
			fputc(value, fin);
		}
		//写匹配文件

		string configFile = filename += ".config";
		FILE* fconfig = fopen(configFile.c_str(),"wb");
		string line;
		for (int i = 0; i < 256; i++)
		{
			if (_infos[i]._count>0)
			{
				line += _infos[i]._ch;
				line += ",";
				char buf[1024];
				_itoa(_infos[i]._count, buf, 10);
				line += buf;
				line += '\n';
				fputs(line.c_str(), fconfig);
			}
			line.clear();
		}

		fclose(fout);
		fclose(fin);
		fclose(fconfig);
	}
	void GenerateHaffManCode(HaffManTreeNode<CharInfo>* root, string code)
	{
		if (root == NULL)
			return;
		if (root->_left == NULL && root->_right == NULL)
		{
			_infos[root->_weight._ch]._code = code;
			return;
		}
		GenerateHaffManCode(root->_left, code + '0');
		GenerateHaffManCode(root->_right, code + '1');

	}
	bool ReadLine(FILE* fout,string& line)
	{
		char ch = fgetc(fout);
		if (ch==EOF)
		{
			return false;
		}
		while (ch!=EOF && ch !='\n')
		{
			line += ch;
			ch = fgetc(fout);
		}
		return true;
	}
	void UnCompress(string filename)
	{
		//读配置文件
		string configname = filename + ".config";
		FILE* fconfig = fopen(configname.c_str(),"rb");
		assert(fconfig);
		string line;

		while(ReadLine(fconfig, line))
		{
			if (line.empty())
			{
				line += '\n';
			}
			else
			{
				unsigned char ch = line[0];
				_infos[ch]._count = atoi(line.substr(2).c_str());
				line.clear();
			}
		}
		CharInfo invalid;
		invalid._count = 0;
		HaffManTree<CharInfo> tree(_infos, 256, invalid);
		string compressFile = filename + ".haffman";

		FILE *fout = fopen(compressFile.c_str(), "rb");
		assert(fout);
		string uncompress = filename += "com";
		FILE *fin = fopen(uncompress.c_str(), "wb");
		unsigned char ch = fgetc(fout);
		int pos = 7;
		HaffManTreeNode<CharInfo>* root = tree.GetRoot();
		HaffManTreeNode<CharInfo>* cur = root;
		int count = root->_weight._count;
		while (ch != EOF)
		{
			if (ch & (1 << pos))
			{
				cur = cur->_right;
			}
			else
				cur = cur->_left;
			if (cur->_left == NULL && cur->_right == NULL)
			{
				fputc(cur->_weight._ch, fin);
				count--;
				cur = root;
			}

			if (pos == 0)
			{
				ch = fgetc(fout);
				pos = 8;
			}
			pos--;
			if (count == 0)
				break;
		}

		fclose(fout);
		fclose(fin);
		fclose(fconfig);
	}
protected:
	CharInfo _infos[256];
};

void TestCompressFile()
{
	FileCompress file;
	file.Compress("a.txt");
}
void TestUnCompressFile()
{
	FileCompress file;
	file.UnCompress("a.txt");
}

