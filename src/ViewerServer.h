#pragma once

#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <thread>

class CalChartDoc;

namespace CalChart {
class Show;
}

/**
 * ViewerServer manages an HTTP server that serves the CalChart Viewer interface
 * and provides an API for accessing the current show data.
 *
 * The server runs on localhost on a configurable port and serves:
 * - /api/show - JSON representation of the current show
 * - / - The viewer web interface (static files from calchart-viewer)
 */
class ViewerServer {
public:
    ViewerServer();
    ~ViewerServer();

    ViewerServer(const ViewerServer&) = delete;
    ViewerServer& operator=(const ViewerServer&) = delete;

    /**
     * Start the HTTP server on the given port.
     * If the server is already running, this does nothing.
     */
    void Start(int port = 8888);

    /**
     * Stop the HTTP server.
     */
    void Stop();

    /**
     * Check if the server is currently running.
     */
    [[nodiscard]] bool IsRunning() const;

    /**
     * Get the port the server is running on.
     */
    [[nodiscard]] int GetPort() const;

    /**
     * Set the current document to be served by the /api/show endpoint.
     * Pass nullptr to clear the current document.
     */
    void SetCurrentDoc(CalChartDoc* doc);

    /**
     * Get the URL at which the viewer can be accessed.
     */
    [[nodiscard]] std::string GetViewerUrl() const;

private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};
