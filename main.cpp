#include <napi.h>
#include <thread>
#if (defined _WIN32 || defined WIN32)
#include <windows.h>
#endif

// Synchronous call
Napi::Number Add(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() != 2)
        throw Napi::TypeError::New(env, "Wrong number of arguments");

    if (!info[0].IsNumber() || !info[1].IsNumber())
        throw Napi::TypeError::New(env, "Wrong arguments");

    const int ret = info[0].ToNumber().Int32Value() + info[1].ToNumber().Int32Value();

    return Napi::Number::New(env, ret);
}

// Return array.
Napi::Array GetFileList(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    Napi::Array result = Napi::Array::New(env);
    for (size_t i = 0; i < 3; i++) {
        Napi::Object obj = Napi::Object::New(env);
        obj.Set(Napi::String::New(env, "filePath"), Napi::String::New(env, "/root/" + std::to_string(i) + ".txt"));
        obj.Set(Napi::String::New(env, "fileSize"), Napi::Number::New(env, i * 100));

        result.Set(Napi::Number::New(env, i), obj);
    }

    return result;
}

// Asynchronous call with callback, based on Napi::AsyncWorker.
// Callback(err, result)
// If N > 0, err is null, result is N ^ 10
// If N <= 0, err is not null and result is null
class Power10AsyncWorker : public Napi::AsyncWorker {
   public:
    Power10AsyncWorker(const Napi::Function& callback, int32_t n) :
        Napi::AsyncWorker(callback),
        n_(n) {
    }

    void Execute() override {
        if (n_ <= 0) {
            SetError("N must larger than 0");
            return;
        }

        result_ = 1;
        for (int i = 0; i < 10; i++) {
            // Simulate time-consuming operations
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            result_ *= n_;
        }
    }

    void OnOK() override {
        Callback().Call({Env().Null(), Napi::Value::From(Env(), result_)});
    }

    void OnError(const Napi::Error& e) override {
        Callback().Call({e.Value(), Env().Null()});
    }

   private:
    int32_t n_ = 0;
    int32_t result_ = 0;
};

void GetPower10(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() != 2)
        throw Napi::TypeError::New(env, "Wrong number of arguments");

    if (!info[0].IsNumber() || !info[1].IsFunction())
        throw Napi::TypeError::New(env, "Wrong arguments");

    int32_t n = info[0].ToNumber().Int32Value();
    Napi::Function callback = info[1].As<Napi::Function>();

    (new Power10AsyncWorker(callback, n))->Queue();
}

// Asynchronous call and return promise.
// If N > 0, Promise.resolve(N^20)
// If N <= 0, Promise.reject(err)
class Power20AsyncWorker : public Napi::AsyncWorker {
   public:
    Power20AsyncWorker(const Napi::Env& env, const Napi::Promise::Deferred& deferred, int32_t n) :
        Napi::AsyncWorker(env),
        deferred_(deferred),
        n_(n) {
    }

    void Execute() override {
        if (n_ <= 0) {
            SetError("N must larger than 0");
            return;
        }

        result_ = 1;
        for (int i = 0; i < 20; i++) {
            // Simulate time-consuming operations
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            result_ *= n_;
        }
    }

    void OnOK() override {
        deferred_.Resolve(Napi::Number::New(Env(), result_));
    }

    void OnError(const Napi::Error& e) override {
        deferred_.Reject(e.Value());
    }

   private:
    Napi::Promise::Deferred deferred_;
    int32_t n_ = 0;
    int32_t result_ = 0;
};

Napi::Value GetPower20(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() != 1)
        throw Napi::TypeError::New(env, "Wrong number of arguments");

    if (!info[0].IsNumber())
        throw Napi::TypeError::New(env, "Wrong arguments");

    int32_t n = info[0].ToNumber().Int32Value();
    Napi::Function callback = info[1].As<Napi::Function>();

    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

    (new Power20AsyncWorker(env, deferred, n))->Queue();

    return deferred.Promise();
}

// Asynchronous call with callback
// call callback function in native thread based on Napi::ThreadSafeFunction.
// Callback(err, result)
// If N > 0, err is null, result is N ^ 30
// If N <= 0, err is not null and result is null.
void GetPower30(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() != 2)
        throw Napi::TypeError::New(env, "Wrong number of arguments");

    if (!info[0].IsNumber() || !info[1].IsFunction())
        throw Napi::TypeError::New(env, "Wrong arguments");

    int32_t n = info[0].ToNumber().Int32Value();
    Napi::Function callback = info[1].As<Napi::Function>();

    std::thread* nativeThread = nullptr;

    Napi::ThreadSafeFunction tsfn = Napi::ThreadSafeFunction::New(
        env,
        callback,
        "Resource Name",             // Name
        0,                           // Unlimited queue
        1,                           // Only one thread will use this initially
        [nativeThread](Napi::Env) {  // Finalizer used to clean threads up
            if (nativeThread && nativeThread->joinable()) {
                nativeThread->join();
            }
        });

    nativeThread = new std::thread([n, tsfn]() {
        struct CallParam {
            std::string error;
            int32_t result = 0;
        };

        CallParam* param = new CallParam();

        if (n > 0) {
            param->result = 1;
            for (int i = 0; i < 30; i++) {
                param->result *= n;
            }
        }
        else {
            param->error = "N must larger than 0";
        }

        auto callback = [](Napi::Env env, Napi::Function jsCallback, CallParam* param) {
            if (jsCallback && param) {
                try {
                    if (param->error.empty()) {
                        jsCallback.Call({env.Null(), Napi::Number::New(env, param->result)});
                    }
                    else {
                        Napi::Error err = Napi::Error::New(env, param->error);
                        jsCallback.Call({err.Value(), env.Null()});
                    }
                } catch (std::exception& e) {
#if (defined _WIN32 || defined WIN32)
                    OutputDebugStringA(e.what());
#endif
                }
            }

            if (param) {
                delete param;
            }
        };

        napi_status status = tsfn.BlockingCall(param, callback);
        if (status != napi_ok) {
            // TODO How to handle error
        }

        tsfn.Release();
    });
}

// Export functions
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "Add"), Napi::Function::New(env, Add));
    exports.Set(Napi::String::New(env, "GetFileList"), Napi::Function::New(env, GetFileList));
    exports.Set(Napi::String::New(env, "GetPower10"), Napi::Function::New(env, GetPower10));
    exports.Set(Napi::String::New(env, "GetPower20"), Napi::Function::New(env, GetPower20));
    exports.Set(Napi::String::New(env, "GetPower30"), Napi::Function::New(env, GetPower30));

    return exports;
}

NODE_API_MODULE(addon, Init)
