#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "timer.h"

namespace py = pybind11;

PYBIND11_MODULE(timer_python_module, m) {
    m.doc() = "Python binding for Timer class";

    py::enum_<TimerOperation>(m, "TimerOperation")
        .value("Disabled", TimerOperation::Disabled)
        .value("Chrome", TimerOperation::Chrome)
        .value("Firefox", TimerOperation::Firefox)
        .value("CSV", TimerOperation::CSV)
        .export_values();

    py::class_<Timer>(m, "Timer")
        .def(py::init<const std::string &, TimerOperation>(), py::arg("outputPath"), py::arg("operation") = TimerOperation::Chrome)
        .def("setOperation", &Timer::setOperation)
        .def("getOperation", &Timer::getOperation)
        .def("addCounterEvent", &Timer::addCounterEvent, 
             py::arg("event_category"), py::arg("event_name"), py::arg("value"), py::arg("args") = std::unordered_map<std::string, std::string>{})
        .def("start", &Timer::start, 
             py::arg("event_category"), py::arg("event_name"), py::arg("args") = std::unordered_map<std::string, std::string>{}, py::arg("measureMemory") = true)
        .def("stop", &Timer::stop, 
             py::arg("event_category"), py::arg("event_name"), py::arg("args") = std::unordered_map<std::string, std::string>{}, py::arg("measureMemory") = true)
        .def("dumpLogs", &Timer::dumpLogs);
}
