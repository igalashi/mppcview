/*
 *
 *
 */

#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

class mPeriod {
	public:
		#if 0
		mPeriod();
		virtual ~mPeriod();
		#endif
		int reset();
		double tap();
		double get_mean() {return m_mean;};
	protected:
	private:
		double mean();

		struct timeval m_at_start;
		struct timeval m_now;
		long long int m_ntap;
		double m_mean;
};

#if 0
mPeriod::mPeriod()
{
	return;
}

mPeriod::~mPeriod()
{
	std::cout << "# Period destructed" << std::endl;
	return;
}
#endif

int mPeriod::reset()
{
	int ret = gettimeofday(&m_at_start, NULL);
	if (ret != 0) perror("mPeriod::reset");
	m_ntap = 0;
	m_mean = 0;
	
	return ret;
}

double mPeriod::tap()
{
	int ret = gettimeofday(&m_now, NULL);
	if (ret != 0) {
		perror("mPeriod::tap");
		return ret;
	}
	m_ntap++;

	double val = mean();
	m_at_start.tv_usec = m_now.tv_usec;
	m_at_start.tv_sec = m_now.tv_sec;
	
	return val;
}

double mPeriod::mean()
{
	int dusec =m_now.tv_usec - m_at_start.tv_usec;
	int dsec = m_now.tv_sec - m_at_start.tv_sec;
	int diff = dsec * 1000 + (dusec / 1000);

	//std::cout << "#D " << m_ntap << " mean: " << m_mean << " diff: " << diff;

	if (m_ntap > 1) {
		m_mean = (m_mean * (m_ntap - 1) + diff) / m_ntap;
	} else
	if (m_ntap == 1) {
		m_mean = static_cast<double>(diff);
	} else {
		m_mean = 0;
	}

	//std::cout << " mean af: " << m_mean << std::endl;

	return m_mean ;
}

#ifdef PERIOD_TEST_MAIN
#include <cstdlib>
int main(int argc, char *argv[])
{
	mPeriod p;
	p.reset();
	for (int i = 0 ; i < 200 ; i++) {
		usleep(10000
			+ ((static_cast<double>(rand()) / RAND_MAX) * 10000));
		double mean = p.tap();
		std::cout << "period : " << mean << std::endl;;
	}

	return 0;
}
#endif

