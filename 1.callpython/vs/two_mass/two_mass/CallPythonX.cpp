#include "CallPythonX.h"



CCallPythonX::CCallPythonX()
{
	m_pFunc = nullptr;
}

CCallPythonX::~CCallPythonX()
{
	if (m_pFunc != nullptr)
	{
		Py_DECREF(m_pFunc);
	}
	Py_Finalize();//�رս�����
}

bool CCallPythonX::init_python() {
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
	return true;
}

bool CCallPythonX::test(std::vector<double> y0, std::vector<double> params, double t0, double T, int N,
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

std::string CCallPythonX::PythonException()
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



