#include "ObjLoader.h"
#include <string>
#include <vector>

std::string __digits1 = "0123456789-.";
int ReadDouble(std::string &s, int *pos, double *value) {
	int start_pos = s.find_first_of(__digits1, *pos);
	int end_pos = s.find_first_not_of(__digits1, start_pos);

	if (start_pos == std::string::npos) return 0;
	if (end_pos == std::string::npos) end_pos = s.size();
	std::string dbl_str = s.substr(start_pos, end_pos - start_pos);
	*value = atof(dbl_str.c_str());
	*pos = end_pos;
	return 1;
}

std::string __digits2 = "0123456789";
int ReadUInt(std::string &s, int *pos, unsigned int *value) {
	int start_pos = s.find_first_of(__digits2, *pos);
	int end_pos = s.find_first_not_of(__digits2, start_pos);

	if (start_pos == std::string::npos) return 0;
	if (end_pos == std::string::npos) end_pos = s.size();
	std::string dbl_str = s.substr(start_pos, end_pos - start_pos);
	*value = atol(dbl_str.c_str());
	*pos = end_pos;
	return 1;
}

int loadModel(char *filename, ObjFile *file) {
	char *buf;
	long buf_size;

	//��������� ����
	HANDLE _file = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	LARGE_INTEGER size;
	//������ ������ �����
	GetFileSizeEx(_file, &size);

	buf_size = size.LowPart;
	buf = new char[buf_size];
	DWORD nBytesRead = 0;

	ReadFile(_file, buf, buf_size, &nBytesRead, 0);
	CloseHandle(_file);

	//vector<string> objStrings;
	char *cur;
	char *new_cur;
	cur = buf;

	std::vector<ObjVertex> V;
	std::vector<ObjTexCord> VT;
	std::vector<ObjNormal> VN;
	std::vector<ObjFace> F;

	bool isHaveTexCoords = false;
	bool isHaveNormals = false;
	std::string str;

	DWORD tick2 = GetTickCount();

	while (1) {
		new_cur = strstr(cur, "\n");
		if (!new_cur) new_cur = buf + buf_size;
		int strsize = new_cur - cur;

		if (strsize > 0) {
			str.clear();
			str.append(cur, strsize);
			//objStrings.push_back(_new);
			while (1) {
				size_t pos = 0;

				// �������� �������
				if (str.find("v ") == 0) {
					DWORD tick1 = GetTickCount();
					int pos = 2;
					ObjVertex vert;
					double v;

					//X
					if (ReadDouble(str, &pos, &v)) {
						vert.x = v;
					} else return -1;

					// Y
					if (ReadDouble(str, &pos, &v)) {
						vert.y = v;
					} else return -2;

					// Z
					if (ReadDouble(str, &pos, &v)) {
						vert.z = v;
					} else return -3;

					// W = 1 def
					if (ReadDouble(str, &pos, &v)) vert.w = v;

					vert.w = 1;
					V.push_back(vert);

					break;
				}

				// �������� �����. ���������
				if (str.find("vt ") == 0) {
					DWORD tick1 = GetTickCount();

					int pos = 2;
					ObjTexCord tex;
					double v;

					if (ReadDouble(str, &pos, &v)) {//u
						tex.u = v;
					} else return -4;

					if (ReadDouble(str, &pos, &v)) {//v
						tex.v = v;
					} else return -5;

					if (ReadDouble(str, &pos, &v)) tex.w = v; //w   =   0   def
						
					tex.w = 0;
					VT.push_back(tex);

					break;
				}

				// �������� ��������
				if (str.find("vn ") == 0) {
					DWORD tick1 = GetTickCount();

					int pos = 2;
					double v;
					ObjNormal norm;

					// N - X
					if (ReadDouble(str, &pos, &v)) {
						norm.x = v;
					} else return -6;

					// N - Y
					if (ReadDouble(str, &pos, &v)) {
						norm.y = v;
					} else return -7;

					// N - Z
					if (ReadDouble(str, &pos, &v)) { 
						norm.z = v;
					} else return -8;

					VN.push_back(norm);

					break;
				}

				// �������� ������
				if (str.find("f ") == 0) {
					//DWORD tick1 = GetTickCount();

					bool isHaveTexCoords = false;
					bool isHaveNormals = false;

					unsigned int d1; // ��������� ����� ��� ������ �����
					int pos = 1;
					std::vector<unsigned int> Vertexes;
					std::vector<unsigned int> TexCoords;
					std::vector<unsigned int> Normals;

					unsigned int v = 0, t = 0, n = 0;

					file->Faces.push_back(ObjFace());
					std::list<ObjFace>::reverse_iterator it = file->Faces.rbegin();

					while (ReadUInt(str, &pos, &d1)) {
						v = d1; // �����
						t = 0;
						n = 0;
						it->vertex.push_back(V[v - 1]);

						if (str[pos] == ' ') break;
						if (str[pos] == '/' && str[pos + 1] == '/') {
							// ������� ��� ��������
							if (ReadUInt(str, &pos, &d1)) {	
								n = d1;
							} else return -9;
						} else if (str[pos] == '/') {
							// ������ ��������
							if (ReadUInt(str, &pos, &d1)) {
								t = d1;
							} else return -10;

							if (str[pos] == ' ') break;

							// �������, ��� ������� ������� ��������
							if (ReadUInt(str, &pos, &d1)) {
								n = d1;
							} else return -11;
						}

						if (n != 0) {
							isHaveNormals = true;
							it->normal.push_back(VN[n - 1]);
						}

						if (t != 0) {
							isHaveTexCoords = true;
							it->texCoord.push_back(VT[t - 1]);
						}

						if (isHaveNormals && (Vertexes.size() != Normals.size()) ||
							isHaveTexCoords && Vertexes.size() != TexCoords.size()) {
							return -12; // ������������ �������� �������! ������		
						}
					}
					break;
				}
				break;
			}
		}
		cur = new_cur + 1;

		if (cur > (buf + buf_size))	break;
	}
	delete buf;

	glDeleteLists(file->listId, 1);
	file->listId = glGenLists(1);

	glNewList(file->listId, GL_COMPILE);
	file->RenderModel(GL_POLYGON);
	glEndList();

	return 1;
}