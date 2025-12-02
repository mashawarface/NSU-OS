echo "testing with mutex, size = 100"
time timeout -s SIGINT 20s ./build/threads_mutex 100

echo "testing with mutex, size = 1000"
time timeout -s SIGINT 20s ./build/threads_mutex 1000

echo "testing with mutex, size = 10000"
time timeout -s SIGINT 20s ./build/threads_mutex 10000

echo "testing with mutex, size = 100000"
time timeout -s SIGINT 20s ./build/threads_mutex 100000


echo "testing with rwlock, size = 100"
time timeout -s SIGINT 20s ./build/threads_rwlock 100

echo "testing with rwlock, size = 1000"
time timeout -s SIGINT 20s ./build/threads_rwlock 1000

echo "testing with rwlock, size = 10000"
time timeout -s SIGINT 20s ./build/threads_rwlock 10000

echo "testing with rwlock, size = 100000"
time timeout -s SIGINT 20s ./build/threads_rwlock 100000


echo "testing with spin, size = 100"
time timeout -s SIGINT 20s ./build/threads_spin 100

echo "testing with spin, size = 1000"
time timeout -s SIGINT 20s ./build/threads_spin 1000

echo "testing with spin, size = 10000"
time timeout -s SIGINT 20s ./build/threads_spin 10000

echo "testing with spin, size = 100000"
time timeout -s SIGINT 20s ./build/threads_spin 100000



