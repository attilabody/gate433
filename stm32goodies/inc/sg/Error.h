/*
 * base.h
 *
 *  Created on: Feb 9, 2018
 *      Author: compi
 */

#ifndef INC_SG_ERROR_H_
#define INC_SG_ERROR_H_
#include <utility>

namespace sg {
class ErrorBase {};
template <typename T> class Error : public ErrorBase
{
public:
	Error(const T &err) : m_e(err) {}
	Error(T &&err) { m_e = std::move(err); }
	Error(const Error &other) : m_e(other.m_e) {}
	Error(Error &&other) { m_e = std::move(other.m_e); }
	const T& GetError() { return m_e; }
	operator const T&() { return m_e; }
private:
	T	m_e;
};

template<typename ErrorType, typename ResultType> class Result
{
public:
	Result() : m_isError(true), m_e() {}
	Result(const ResultType &r) : m_isError(false), m_r(r) {}
	Result(const ErrorType &e) : m_isError(true), m_e(e) {}
	Result(const Result &other) : m_isError(other.m_isError) {
		if(m_isError) new(&m_e) ErrorType(other.m_e);
		else new(&m_r) ResultType(other.m_r);
	}
	bool IsError() { return m_isError; }
	Result<ErrorType, ResultType>& operator=(const Result<ErrorType, ResultType>& other) {
		if(m_isError) {
			if(other.m_isError)
				m_e = other.m_e;
			else {
				m_e.~ErrorType();
				new(&m_r) ResultType(other.m_r);
			}
		} else {
			if(other.m_isError) {
				m_r.~ResultType();
				new(&m_e) ErrorType(other.m_e);
			} else
				m_r = other.m_r;
		}
		return *this;
	}
	const ErrorType& GetError() { return m_isError ? m_e : *((ResultType*)nullptr); }
	const ResultType& GetResult() { return m_isError ? *((ErrorType*)nullptr) : m_r; }
	operator const ErrorType&() { return GetError(); }
	operator const ResultType&() { return GetResult(); }

private:
	bool m_isError;
	union {
		ErrorType m_e;
		ResultType m_r;
	};
};

}
#endif /* INC_SG_ERROR_H_ */
