#define BOOST_TEST_MODULE test

#include <boost/test/unit_test.hpp>

#include "recycle.hpp"
#include <boost/thread.hpp>
#include <fstream>

template <std::size_t S> struct test {
	char buffer[S];
};

template <std::size_t S> struct suite {
	typedef test<S> test_type;
	void test_my() {
		using namespace recycle;
		int count = (1024 * 1024);
		for (int i = 0; i < count; ++i) {
			auto foo1 = make_unique<test_type>();
		}
	}

	void test_std() {
		using namespace std;
		int count = (1024 * 1024);
		for (int i = 0; i < count; ++i) {
			auto foo1 = make_unique<test_type>();
		}
	}

	static void benchmark(unsigned short thread_count = 4) {

		suite<S> s;
		s.thread_count = thread_count;

		int count = 10;
		for (int i = 0; i < count; i++) {
			s.run();
		}
		std::cout << S << "\t" << s.my / count << "\t" << s.std / count << std::endl;
	}

	void run() {
		using namespace std::chrono;

		boost::thread_group threads;

		auto start = high_resolution_clock::now();
		for (int i = 0; i < thread_count; i++) {
			threads.create_thread([&] { test_my(); });
		}
		threads.join_all();
		auto end = high_resolution_clock::now();
		auto dur = duration_cast<milliseconds>(end - start);
		my += dur.count();

		start = high_resolution_clock::now();
		for (int i = 0; i < thread_count; i++) {
			threads.create_thread([&] { test_std(); });
		}
		threads.join_all();
		end = high_resolution_clock::now();
		dur = duration_cast<milliseconds>(end - start);
		std += dur.count();
	}

	unsigned long my = 0;
	unsigned long std = 0;
	unsigned short thread_count = 1;
};



BOOST_AUTO_TEST_CASE(unique_test) {
	int* d = nullptr;
	{
		auto ptr = recycle::make_unique<int>(23);
		d = ptr.get();
	}
	auto ptr = recycle::make_unique<int>(42);
	BOOST_CHECK_EQUAL(d, ptr.get());
}

BOOST_AUTO_TEST_CASE(shared_test) {
	int* d = nullptr;
	{
		auto ptr = recycle::make_shared<int>(23);
		d = ptr.get();
	}
	auto ptr = recycle::make_shared<int>(42);
	BOOST_CHECK_EQUAL(d, ptr.get());

}


BOOST_AUTO_TEST_CASE(scalability_test) {
	std::remove("scalability_test.dat");
	std::ofstream out("scalability_test.dat");
	std::streambuf *coutbuf = std::cout.rdbuf();
	std::cout.rdbuf(out.rdbuf());
	for (int i = 1; i < 5; ++i) {
		std::cout << i << " ";
		suite<1024>::benchmark(i);
	}
	std::cout.rdbuf(coutbuf);
}

BOOST_AUTO_TEST_CASE(object_size_test) {
	std::remove("object_size_test.dat");
	std::ofstream out("object_size_test.dat");
	std::streambuf *coutbuf = std::cout.rdbuf();
	std::cout.rdbuf(out.rdbuf());
	suite<4>::benchmark();
	suite<8>::benchmark();
	suite<16>::benchmark();
	suite<32>::benchmark();
	suite<64>::benchmark();
	suite<128>::benchmark();
	suite<256>::benchmark();
	suite<512>::benchmark();
	suite<1024>::benchmark();
	suite<2048>::benchmark();
	suite<4096>::benchmark();
	std::cout.rdbuf(coutbuf);
}
