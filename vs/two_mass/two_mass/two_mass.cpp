// two_mass.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <vector>
#include <string>
#include "Python.h"
#include "frameobject.h"



class CCallPython {
public:
	CCallPython();
	~CCallPython();

	bool init_python();
	bool test(std::vector<double> y0, std::vector<double> params, double t0, double T, int N,
		std::vector<std::vector<double>> &ret);
	std::string PythonException();
private:
	PyObject *m_pFunc;

};


int main()
{
	std::vector<double> y0;
	std::vector<double> params;
	std::vector<std::vector<double>> ret;

	float po = 1 / 4.0;           // which power law
	int Q = 100;                // scaling factor with respect to mass
	double dr = 0.00001;        // dampling ratio
	double m1l = 0.01 * Q, m1r = 0.01 * Q, m1 = 0.01 * Q;
	double m2l = 0.005 * Q, m2r = 0.005 * Q, m2 = 0.005 * Q;
	double k1l = 5 * pow(Q, po), k1r = 5 * pow(Q, po), k1 = 5 * pow(Q, po);
	double k2l = 2 * pow(Q, po), k2r = 2 * pow(Q, po), k2 = 2 * pow(Q, po);
	double kcl = 2 * pow(Q, po), kcr = 2 * pow(Q, po), kc = 2 * pow(Q, po);
	double c1l = 3 * k1, c1r = 3 * k1, c1 = 3 * k1;
	double c2l = 3 * k2, c2r = 3 * k2, c2 = 3 * k2;
	double r1l = 2 * dr * pow((m1 * k1), 0.5), r1r = 2 * dr * pow((m1 * k1), 0.5), r1 = 2 * dr * pow((m1 * k1), 0.5);
	double r2l = 2 * dr * pow((m2 * k2), 0.5), r2r = 2 * dr * pow((m2 * k2), 0.5), r2 = 2 * dr * pow((m2 * k2), 0.5);
	double a10 = 0.05 * pow(Q, (2 * po)), a20 = 0.05 * pow(Q, (2 * po));
	double l = 0.3 * pow(Q, po);
	double d1 = 0.1 * pow(Q, po);
	double d2 = 0.05 * pow(Q, po);
	double Ps = 0.05;

	// Initialization
	double x1l0 = 0.1, x1r0 = 0.1;
	double y1l0 = 0.1, y1r0 = 0.1;
	double x2l0 = 0.1, x2r0 = 0.1;
	double y2l0 = 0, y2r0 = 0;

	// Bundle params
	params = { m1l, m2l, m1r, m2r, k1l, k2l, k1r, k2r,r1l, r2l, r1r, r2r, c1l, c2l, c1r, c2r,kcl, kcr, a10, a20, d1, d2, l, Ps };
	// Bundle initial conditions
	y0 = { x1l0, y1l0, x2l0, y2l0, x1r0, y1r0, x2r0, y2r0 };

	CCallPython call;
	call.init_python();
	clock_t s =  clock();
	call.test(y0, params, 0, 50, 10000, ret);
	clock_t dif = clock()-s;

    return 0;
}

CCallPython::CCallPython()
{
	m_pFunc = nullptr;
}

CCallPython::~CCallPython()
{
	if (m_pFunc != nullptr)
	{
		Py_DECREF(m_pFunc);
	}
	Py_Finalize();//�رս�����
}

bool CCallPython::init_python() {
	//����python��װ·��
	Py_SetPythonHome(L"D:/python/Anaconda3"); //TODO
	//��ʼ��������
	Py_Initialize();

	// ����ʼ���Ƿ�ɹ�
	if (!Py_IsInitialized())
	{
		return false;
	}

	//���python�ļ�����Ŀ¼
	char chBuffer[1024] = { 0 };
	sprintf_s((char*)chBuffer, 1024, "sys.path.append('%s')", "D:/Users/bob.li/Desktop/python");//TODO
	PyRun_SimpleString("import sys");
	PyRun_SimpleString(chBuffer);

	//ģ��
	PyObject *pModul = PyImport_ImportModule("two_mass_model");//�ű����ƣ��޺�׺ TODO
	if (pModul == nullptr)
	{
		Py_Finalize();//�رս�����
		return false;
	}

	//����
	m_pFunc = PyObject_GetAttrString(pModul, "func_odeint");//�ӿ��� TODO
	Py_DECREF(pModul);
	if (m_pFunc == nullptr)
	{
		Py_Finalize();//�رս�����
		return false;
	}
}

bool CCallPython::test(std::vector<double> y0, std::vector<double> params, double t0, double T, int N,
	std::vector<std::vector<double>> &ret)
{
	//����
	PyObject *pArgs = nullptr;
	//����tuple���ݲ���
	pArgs = PyTuple_New(5); //5������
	if (pArgs == nullptr)
	{
		Py_DECREF(m_pFunc);
		Py_Finalize();//�رս�����
		return false;
	}
	//����һ y0
	PyObject *PyListy0 = PyList_New(y0.size());
	for (int i = 0; i < y0.size(); i++)
	{
		PyList_SetItem(PyListy0, i, PyFloat_FromDouble(y0.at(i)));//��PyList�����ÿ��Ԫ�ظ�ֵ
	}
	PyTuple_SetItem(pArgs, 0, PyListy0);
	//������ params
	PyObject *PyListparams = PyList_New(params.size());
	for (int i = 0; i < params.size(); i++)
	{
		PyList_SetItem(PyListparams, i, PyFloat_FromDouble(params.at(i)));//��PyList�����ÿ��Ԫ�ظ�ֵ
	}
	PyTuple_SetItem(pArgs, 1, PyListparams);
	//������ t0
	PyTuple_SetItem(pArgs, 2, PyFloat_FromDouble(t0));
	//������ T
	PyTuple_SetItem(pArgs, 3, PyFloat_FromDouble(T));
	//������ N
	PyTuple_SetItem(pArgs, 4, PyLong_FromLong(N));

	//ִ�к���
	PyObject *pRet = PyObject_CallObject(m_pFunc, pArgs);
	Py_DECREF(pArgs);
	Py_DECREF(PyListy0);
	Py_DECREF(PyListparams);
	if (pRet == nullptr) {
		std::string str = PythonException();
		Py_Finalize();//�رս�����
		return false;
	}
	//��������ֵ ��άlist

	//��ȡ����ֵ---ͨ���ڸ������������չ�ӿڻ�ȡ��
	int SizeOfList = PyList_Size(pRet);//List����Ĵ�С������SizeOfList = 
	for (int i = 0; i < SizeOfList; i++) {
		PyObject *Item = PyList_GetItem(pRet, i);//��ȡList�����е�ÿһ��Ԫ��
		std::vector<double> subvec;
		int subsize = PyList_Size(Item);
		for (int j = 0; j < subsize; j++)
		{
			PyObject *subItem = PyList_GetItem(Item, j);
			double d;
			PyArg_Parse(subItem, "d", &d);//i��ʾת����double�ͱ���
			subvec.push_back(d);
			//Py_DECREF(subItem); 
		}
		ret.push_back(subvec);
		//Py_DECREF(Item); //�ͷſռ�
	}

	
	return true;
}

std::string CCallPython::PythonException()
{
	PyObject *pType = nullptr;
	PyObject *pValue = nullptr;
	PyObject *pTraceBack = nullptr;
	PyObject *pStr = nullptr;
	PyTracebackObject *pTb = nullptr;
	std::string strRet;
	int iByte = 0;

	//����δ֪����
	PyErr_Fetch(&pType, &pValue, &pTraceBack);

	//����
	if (pType != nullptr)
	{
		std::string m_strType = PyExceptionClass_Name(pType);
		m_strType += ":";
	}

	//������Ϣ
	if (pValue != nullptr)
	{
		pStr = PyUnicode_FromFormat("%S", pValue);

		if (pStr != nullptr)
		{
			int i = PyUnicode_READY(pStr);
			iByte = PyUnicode_KIND(pStr);
			assert(iByte == 1);
			if (iByte != 1)
			{
				return "";
			}
			strRet = (char*)PyUnicode_1BYTE_DATA(pStr);
		}
	}

	//��ջ������Ϣ
	if (pTraceBack != nullptr)
	{
		pTb = (PyTracebackObject*)pTraceBack;
		for (; pTb != nullptr; pTb = pTb->tb_next)
		{
			PyObject*p = PyObject_Str(pTb->tb_frame->f_code->co_filename);
			int i = PyUnicode_Check(p);
			i = PyByteArray_Check(p);

			pStr = PyUnicode_FromFormat(" Flie:%S, Line:%d, in:%S\n",
				pTb->tb_frame->f_code->co_filename,
				pTb->tb_lineno,
				pTb->tb_frame->f_code->co_name);
			iByte = PyUnicode_KIND(pStr);
			assert(iByte == 1);
			if (iByte != 1)
			{
				return strRet;
			}
			strRet += (char*)PyUnicode_1BYTE_DATA(pStr);
		}
	}

	//��Դ�ͷ�
	//PyErr_Restore(pType, pValue, pTraceBack);

	Py_XDECREF(pType);
	Py_XDECREF(pValue);
	Py_XDECREF(pTraceBack);

	return strRet;
}


