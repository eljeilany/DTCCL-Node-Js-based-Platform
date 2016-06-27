// addon.cc
#include <node.h>
#include <iostream>
#include <sstream>
#include <nan.h>
#include <node_object_wrap.h>
#include "myobject.h"
#include "DTCCL.h"
#include <b64/encode.h>
#include <b64/decode.h>
#include <v8.h>
using namespace v8;
//,'-fno-exceptions'
class MyWorker : public NanAsyncWorker {
 public:
  std::string inName;
  MyWorker(NanCallback *callback, MyObject* o1,std::string imStr,std::string name, int mode)
    : NanAsyncWorker(callback),o1(o1),imStr(imStr),name(name), mode(mode) {
    }
  ~MyWorker() {}
  // void setN(NumberC ob1, NumberC ob2){
  //   o1 = ob1;
  //   o2 = ob2;
  // }
  // Executed inside the worker-thread.
  // It is not safe to access V8, or V8 data structures
  // here, so everything we need for input and output
  // should go on `this`.
  void Execute () {
  //std::cout << "O2 from worker :" << o2->Num.ret() << std::endl;
    //std::cout << "async good1" << std::endl;
    base64::decoder D;
    std::stringstream instr;
    instr.str(imStr);
    D.decode(instr, imStream) ;
    //std::cout << "async good2" << std::endl;
    std::cout << cv::getBuildInformation() << std::endl;
    std::string someString = imStream.str();
    std::vector<char> someVec(someString.begin(), someString.end());
    input = cv::imdecode(someVec,CV_LOAD_IMAGE_GRAYSCALE);
    //std::cout << "async good3" << std::endl;
    std::stringstream ss ;
    switch (mode) {
      case 0:
      /*** detect ***/
      //std::cout << "async good4" << std::endl;
      output = dtccl.justDetect(input,name);
      //std::cout << "async good5" << std::endl;
      break;
      case 1:
      /** initialise **/
      ss.str(std::string());
      ss<< "{\"name\":\"" << name << "\",\"found\":\"false\"}";
      try{
        DTCCL newdt;
        o1->dtccl = newdt;
        o1->dtccl.set(true,0,0,1);
        im_gray = cv::imread(inName,CV_LOAD_IMAGE_GRAYSCALE);
        o1->dtccl.initialise(im_gray,inR);
        std::cout << "frame Process" << std::endl;
        o1->dtccl.processFrame(input);
        if(o1->dtccl.found){
          ss.str(std::string());
          ss << "{ \"name\":\""<< name << "\",\"found\":\"true\",\"center\":{\"x\":\""<<o1->dtccl.drawc.x
          << "\",\"y\":\""<<std::round(o1->dtccl.drawc.y)
          <<"\"}, \"points\":[ ";
          ss << "{\"x\":\"" << std::round(o1->dtccl.vertices[0].x) <<"\",\"y\":\""<<std::round(o1->dtccl.vertices[0].y)
          <<"\"}";
          for(unsigned i =1;i<4;i++){
            ss << ",{\"x\":\"" << std::round(o1->dtccl.vertices[i].x) <<"\",\"y\":\""<<std::round(o1->dtccl.vertices[i].y)
            <<"\"}";
          }
          ss << "]}";
        }
      }catch(std::exception& e){
        std::cout << e.what() << '\n';
      }
      output = ss.str();
      break;
      case 2:
      /*** track ***/
      ss.str(std::string());
      ss<< "{\"name\":\"" << name << "\",\"found\":\"false\"}";
      try{
        o1->dtccl.processFrame(input);

        if(o1->dtccl.found){
          ss.str(std::string());
          ss << "{ \"name\":\""<< name << "\",\"found\":\"true\",\"center\":{\"x\":\""<<o1->dtccl.drawc.x
          << "\",\"y\":\""<<std::round(o1->dtccl.drawc.y)
          <<"\"}, \"points\":[ ";
          ss << "{\"x\":\"" << std::round(o1->dtccl.vertices[0].x) <<"\",\"y\":\""<<std::round(o1->dtccl.vertices[0].y)
          <<"\"}";
          for(unsigned i =1;i<4;i++){
            ss << ",{\"x\":\"" << std::round(o1->dtccl.vertices[i].x) <<"\",\"y\":\""<<std::round(o1->dtccl.vertices[i].y)
            <<"\"}";
          }
          ss << "]}";
        }

      }catch(std::exception& e){
        std::cout << e.what() << '\n';
      }
      output = ss.str();
      break;
    }

  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use V8 again
  void HandleOKCallback () {
    NanScope();
    //Isolate* isolate = Isolate::GetCurrent();
    Local<Value> argv[] = {
        NanNull()
      , NanNew<String>(output)
    } ;
  //Local<String> str = NanNew<String>(output);
    callback->Call(2, argv);
  }
  void setinr(int x,int y,int w,int h){
    inR = cv::Rect(x,y,w,h);
  }

 private:
  MyObject* o1;
  std::string imStr;
  std::string name;
  int mode;
  cv::Rect inR;
  std::string i_name;
  std::string output;
  std::stringstream imStream;
  DTCCL dtccl;
  cv::Mat im_gray, input;
};

void CreateObject(const FunctionCallbackInfo<Value>& args) {
  std::cout << "creat good1" << std::endl;
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  //std::cout << "creat good2" << std::endl;
  MyObject::NewInstance(args);
}

void Add(const FunctionCallbackInfo<Value>& args) {
  // Isolate* isolate = Isolate::GetCurrent();
  // HandleScope scope(isolate);
  //
  // MyObject* obj1 = node::ObjectWrap::Unwrap<MyObject>(
  //     args[0]->ToObject());
  // MyObject* obj2 = node::ObjectWrap::Unwrap<MyObject>(
  //     args[1]->ToObject());
  // std::cout << "Obj1 value: " << obj1->Num.ret() << std::endl;
  // std::cout << "Obj2 value: " << obj2->Num.ret() << std::endl;
  // double sum = obj1->value() + obj2->value();
  // args.GetReturnValue().Set(Number::New(isolate, sum));
}

NAN_METHOD(Justdetect) {
  NanScope();
  std::cout << "detect called" << std::endl;
  MyObject* obj = node::ObjectWrap::Unwrap<MyObject>(
      args[0]->ToObject());
  Local<String> str = args[1].As<String>();
  std::string im = *String::Utf8Value(str);
  str = args[2].As<String>();
  std::string name = *String::Utf8Value(str);
  //std::cout << "nan good1" << std::endl;
  Local<Function> callback = args[3].As<Function>();
  //int mode = args[4].As<Number>()->IntegerValue();
  NanCallback* nanCallback = new NanCallback(callback);
  MyWorker* worker = new MyWorker(nanCallback, obj,im,name,0);
  // // worker->o1 = obj1->Num;
  // // worker->o2 = obj2->Num;
  NanAsyncQueueWorker(worker);
  //std::cout << "nan good2" << std::endl;
  NanReturnUndefined();
}

NAN_METHOD(Initialize) {
  NanScope();
  std::cout << "Initialize called" << std::endl;
  MyObject* obj = node::ObjectWrap::Unwrap<MyObject>(
      args[0]->ToObject());
  Local<String> str = args[1].As<String>();
  std::string im = *String::Utf8Value(str);
  str = args[2].As<String>();
  std::string name = *String::Utf8Value(str);
  std::cout << "nan good1" << std::endl;
  Local<Function> callback = args[8].As<Function>();
  str = args[3].As<String>();
  NanCallback* nanCallback = new NanCallback(callback);
  MyWorker* worker = new MyWorker(nanCallback, obj,im,name,1);
  // // worker->o1 = obj1->Num;
  // // worker->o2 = obj2->Num;
  worker->inName = *String::Utf8Value(str);
  worker->setinr(args[4].As<Number>()->IntegerValue(),args[5].As<Number>()->IntegerValue(),
  args[6].As<Number>()->IntegerValue(),args[7].As<Number>()->IntegerValue());
  NanAsyncQueueWorker(worker);
  std::cout << "nan good2" << std::endl;
  NanReturnUndefined();
}
NAN_METHOD(Track) {
  NanScope();
  std::cout << "track called" << std::endl;
  MyObject* obj = node::ObjectWrap::Unwrap<MyObject>(
      args[0]->ToObject());
  Local<String> str = args[1].As<String>();
  std::string im = *String::Utf8Value(str);
  str = args[2].As<String>();
  std::string name = *String::Utf8Value(str);
  std::cout << "nan good1" << std::endl;
  Local<Function> callback = args[3].As<Function>();
  NanCallback* nanCallback = new NanCallback(callback);
  MyWorker* worker = new MyWorker(nanCallback, obj,im,name,2);
  // // worker->o1 = obj1->Num;
  // // worker->o2 = obj2->Num;
  NanAsyncQueueWorker(worker);
  std::cout << "nan good2" << std::endl;
  NanReturnUndefined();
}

void InitAll(Handle<Object> exports) {
  MyObject::Init();

  NODE_SET_METHOD(exports, "createObject", CreateObject);
  // NODE_SET_METHOD(exports, "add", Add);
  // NODE_SET_METHOD(exports, "doublev", doubleV);
  exports->Set(NanNew("justdetect"), NanNew<FunctionTemplate>(Justdetect)->GetFunction());
  exports->Set(NanNew("initialize"), NanNew<FunctionTemplate>(Initialize)->GetFunction());
  exports->Set(NanNew("track"), NanNew<FunctionTemplate>(Track)->GetFunction());
}

NODE_MODULE(addon, InitAll)
