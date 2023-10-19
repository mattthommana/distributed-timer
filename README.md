# to use:
if you have a ca-cert file needed by your company put it in the docker/certs folder
```
set your UID and GID as bash variables (I just put it in my bashrc)
cd docker
docker compose --profile run-test up --build
docker compose --profile merge-files up --build
```

---OR---
```
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
```

terminal 1
```
distributed_timer_server
```

terminal 2
```
distributed_timer_client
```
when done you will have 2 files in logs. You need to post process and merge them. 
To do so, run the following
```
distributed_timer_merge_tool ../logs/combined-times.json ../logs/client-times.json ../logs/server-times.json
```

Then go to chrome://tracing and load the file to see the timers and counters.

# to create python bindings
pip install pybind11
cmake -DCMAKE_BUILD_TYPE=Release -DCreatePythonBindings=ON ..
make -j

# About
- Can add counters manually
- Add a custom memory counter
- Categories show up as the thread name and a new row in chrome
- Names show up as the duration name on the bar and make a new color per name


# Bugs
- size in the example is zeroed only in memory counter...?