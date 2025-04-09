#ifndef ULTIMATE_CONTROL_VOLUME_MANAGER_HPP
#define ULTIMATE_CONTROL_VOLUME_MANAGER_HPP

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace Volume {

struct AudioSink {
    std::string name;
    std::string description;
    int volume; // 0-100
    bool muted;
};

class VolumeManager {
public:
    using SinkList = std::vector<AudioSink>;
    using SinkUpdateCallback = std::function<void(const SinkList&)>;

    VolumeManager();
    ~VolumeManager();

    void refresh_sinks();
    void set_volume(const std::string& sink_name, int volume);
    void toggle_mute(const std::string& sink_name);
    void set_update_callback(SinkUpdateCallback cb);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Volume

#endif // ULTIMATE_CONTROL_VOLUME_MANAGER_HPP
