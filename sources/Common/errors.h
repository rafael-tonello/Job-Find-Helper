#ifndef __ERRORS__H__ 
#define __ERRORS__H__ 

#include <string>
#include <Utils.h>

using namespace std;

using Error = string;
class Errors { 
private:
public:
    static Error NoError;
    static Error TimeoutError;
    static Error ConnectionIsNotStablishedError;
    static Error createError(string message){ return message; }
    static Error createError(string message, Error nestedError)
    {
        //return message + ":\n" + "  >" + Utils::stringReplace(nestedError, "\n", "\n  "); 
        return message + ":\n" + "  >" + Utils::stringReplace(nestedError, "\n", "\n  "); 
    }

    static void forNestedErrors(Error errorWithNestedErrors, function<void(Error err)> f)
    {
        while (true)
        {
            auto cutPos = errorWithNestedErrors.find('>');
            if( cutPos != string::npos)
            {
                auto currErr = errorWithNestedErrors.substr(0, cutPos);
                errorWithNestedErrors = errorWithNestedErrors.substr(cutPos +1);
                if (currErr != "")
                    f(currErr);
            }
            else
                break;
        }

        if (errorWithNestedErrors != "")
            f(errorWithNestedErrors);
    }
};

template <typename T>
class ResultWithStatus{
public:
    T result;
    Error status = Errors::NoError;

    ResultWithStatus(){}
    ResultWithStatus(T result, Error error): result(result), status(error){}
};

// class Error2B{};

// class Error2: public Error2B{
// private:
//     string message = "";
// public:
//     Error2B nestedError;

//     Error2(){}
//     Error2(string message):message(message){}
//     Error2(string message, Error2 nestedErrorp);

//     string toString(bool includedNestedErrors = false)
//     {
//         string result = this->message;

//         if (includedNestedErrors)
//         {
//             auto tmp = ((Error2*)&(this->nestedError))->toString(true);
//             if (tmp != "")
//                 result += "\n" + tmp;

//         }

//         return result;

//     }

//     void operator << (string toAppend){ this->message+= toAppend;}
//     void operator + (string toAppend){ this->message+= toAppend;}

//     operator string(){return toString(); }
//     operator const char*(){return toString().c_str(); }


// };
 
#endif 
