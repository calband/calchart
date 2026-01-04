#include "ViewerServer.h"

#include "CalChartViewerHtml.h"
#include <fstream>
#include <httplib.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include "CalChartDoc.h"

class ViewerServer::Impl {
public:
    Impl()
        : mPort(8888)
        , mIsRunning(false)
        , mCurrentDoc(nullptr)
    {
    }

    ~Impl()
    {
        Stop();
    }

    void Start(int port)
    {
        std::lock_guard<std::mutex> lock(mMutex);

        if (mIsRunning) {
            wxLogDebug("ViewerServer: Server already running on port %d", mPort);
            return;
        }

        mPort = port;
        mServer = std::make_unique<httplib::Server>();
        wxLogDebug("ViewerServer: Creating server on port %d", mPort);

        // Route: GET /api/show - returns the current show as JSON
        mServer->Get("/api/show", [this](const httplib::Request&, httplib::Response& res) {
            wxLogDebug("ViewerServer: GET /api/show requested");
            std::lock_guard<std::mutex> showLock(mMutex);

            if (!mCurrentDoc) {
                wxLogDebug("ViewerServer: No current doc loaded");
                res.set_content(R"({"error": "No show loaded"})", "application/json");
                res.status = 400;
                return;
            }

            try {
                wxLogDebug("ViewerServer: Generating show JSON from CalChartDoc...");

                // Get the viewer JSON from the document
                auto response = mCurrentDoc->toViewerJSON();

                res.set_content(response.dump(4), "application/json");
                res.status = 200;
                wxLogDebug("ViewerServer: /api/show response sent successfully");
            } catch (const std::exception& e) {
                wxLogError("ViewerServer: Exception in /api/show: %s", e.what());
                nlohmann::json error;
                error["error"] = e.what();
                res.set_content(error.dump(), "application/json");
                res.status = 500;
            }
        });

        // Route: GET /api/status - health check
        mServer->Get("/api/status", [](const httplib::Request&, httplib::Response& res) {
            wxLogDebug("ViewerServer: GET /api/status requested");
            res.set_content(R"({"status": "ok"})", "application/json");
            res.status = 200;
        });

        // Route: GET / - serve viewer HTML
        mServer->Get("/", [](const httplib::Request&, httplib::Response& res) {
            wxLogDebug("ViewerServer: GET / requested");
            auto html = CalChart::ViewerHtml::GetViewerHtml();
#ifdef CMAKE_VIEWER_SOURCE_DIR
            wxLogDebug("ViewerServer: Serving from filesystem: %s", CMAKE_VIEWER_SOURCE_DIR);
#else
            wxLogDebug("ViewerServer: Serving embedded HTML");
#endif
            res.set_content(html, "text/html");
            res.status = 200;
            wxLogDebug("ViewerServer: Root page served");
        });

        // Route: GET /viewer - same as /
        mServer->Get("/viewer", [](const httplib::Request&, httplib::Response& res) {
            wxLogDebug("ViewerServer: GET /viewer requested");
            auto html = CalChart::ViewerHtml::GetViewerHtml();
            res.set_content(html, "text/html");
            res.status = 200;
        });

#ifdef CMAKE_VIEWER_SOURCE_DIR
        // In debug mode, serve static files from viewer directory
        // We explicitly serve files instead of using set_mount_point to avoid
        // ASAN crashes in httplib's conditional request handling
        mServer->Get(R"(.+\.(css|js|png|jpg|jpeg|gif|svg|ico|json|woff|woff2|ttf|eot))", [](const httplib::Request& req, httplib::Response& res) {
            auto path = std::string(CMAKE_VIEWER_SOURCE_DIR) + req.path;
            std::ifstream file(path, std::ios::binary);
            if (file) {
                std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

                // Set content type based on extension
                auto ext = req.path.substr(req.path.find_last_of('.') + 1);
                std::string contentType = "application/octet-stream";
                if (ext == "css")
                    contentType = "text/css";
                else if (ext == "js")
                    contentType = "application/javascript";
                else if (ext == "json")
                    contentType = "application/json";
                else if (ext == "png")
                    contentType = "image/png";
                else if (ext == "jpg" || ext == "jpeg")
                    contentType = "image/jpeg";
                else if (ext == "gif")
                    contentType = "image/gif";
                else if (ext == "svg")
                    contentType = "image/svg+xml";
                else if (ext == "ico")
                    contentType = "image/x-icon";

                res.set_content(content, contentType.c_str());
                res.status = 200;
            } else {
                res.status = 404;
                res.set_content("File not found", "text/plain");
            }
        });
        wxLogDebug("ViewerServer: Configured to serve static files from %s", CMAKE_VIEWER_SOURCE_DIR);
#endif

        // Start the server in a background thread
        mThread = std::thread([this]() {
            wxLogDebug("ViewerServer: Thread started, calling listen() on port %d", mPort);
            bool result = mServer->listen("localhost", mPort);
            wxLogDebug("ViewerServer: listen() returned: %d", result);
        });

        // Give the server a moment to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        mIsRunning = true;
        wxLogDebug("ViewerServer: Server started successfully on localhost:%d", mPort);
    }

    void Stop()
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);

            if (!mIsRunning) {
                return;
            }

            mIsRunning = false;

            // Signal the server to stop, but don't destroy it yet
            if (mServer) {
                mServer->stop();
            }
        }

        // Wait for the thread to finish (outside the lock to avoid deadlock)
        if (mThread.joinable()) {
            mThread.join();
        }

        // Now it's safe to destroy the server
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mServer.reset();
        }
    }

    bool IsRunning() const
    {
        std::lock_guard<std::mutex> lock(mMutex);
        return mIsRunning;
    }

    int GetPort() const
    {
        std::lock_guard<std::mutex> lock(mMutex);
        return mPort;
    }

    void SetCurrentDoc(CalChartDoc* doc)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mCurrentDoc = doc;
    }

private:
    mutable std::mutex mMutex;
    std::unique_ptr<httplib::Server> mServer;
    std::thread mThread;
    int mPort;
    bool mIsRunning;
    CalChartDoc* mCurrentDoc;
};

ViewerServer::ViewerServer()
    : mImpl(std::make_unique<Impl>())
{
}

ViewerServer::~ViewerServer() = default;

void ViewerServer::Start(int port)
{
    mImpl->Start(port);
}

void ViewerServer::Stop()
{
    mImpl->Stop();
}

bool ViewerServer::IsRunning() const
{
    return mImpl->IsRunning();
}

int ViewerServer::GetPort() const
{
    return mImpl->GetPort();
}

void ViewerServer::SetCurrentDoc(CalChartDoc* doc)
{
    mImpl->SetCurrentDoc(doc);
}

std::string ViewerServer::GetViewerUrl() const
{
    return "http://localhost:" + std::to_string(GetPort()) + "/";
}
