#pragma once
#include <map>
#include <set>
#include <vector>
#include <deque>

template<typename TKey, typename TValue, typename TFunc>
void diff_maps(const std::map<TKey, TValue>& new_map, const std::map<TKey, TValue>& old_map, const TFunc& func)
{
	auto new_it = new_map.begin();
	auto old_it = old_map.begin();

	while ((new_it != new_map.end()) || (old_it != old_map.end())) {
		int diff = 0;
		if (new_it != new_map.end()) {
			if (old_it != old_map.end()) {
				if (new_it->first > old_it->first) {
					diff = -1;
				} else if (new_it->first < old_it->first) {
					diff = 1;
				}
			} else diff = 1;
		} else {
			if (old_it != old_map.end()) {
				diff = -1;
			}
		}

		if (diff > 0) {
			func(new_it->first, &new_it->second, nullptr);
			++new_it;
		} else if (diff < 0) {
			func(old_it->first, nullptr, &old_it->second);
			++old_it;
		} else {
			func(old_it->first, &new_it->second, &old_it->second);
			++new_it;
			++old_it;
		}
	}
}

template<typename TValue, typename TFunc>
void diff_sets(const std::set<TValue>& new_set, const std::set<TValue>& old_set, const TFunc& func)
{
	auto new_it = new_set.begin();
	auto old_it = old_set.begin();

	while ((new_it != new_set.end()) || (old_it != old_set.end())) {
		int diff = 0;
		if (new_it != new_set.end()) {
			if (old_it != old_set.end()) {
				if (*new_it > *old_it) {
					diff = -1;
				} else if (*new_it < *old_it) {
					diff = 1;
				}
			} else diff = 1;
		} else {
			if (old_it != old_set.end())
				diff = -1;
		}

		if (diff > 0) {
			func(&*new_it, nullptr);
			++new_it;
		} else if (diff < 0) {
			func(nullptr, &*old_it);
			++old_it;
		} else {
			++new_it;
			++old_it;
		}
	}
}

template<typename TValue, typename TFunc>
void diff_sequences(const std::vector<TValue>& new_seq, const std::vector<TValue>& old_seq, TFunc& func) {
	// code derived from DTL (BSD License) https://github.com/cubicdaiya/dtl

	const bool swapped = (std::size(new_seq) > std::size(old_seq));
	const std::vector<TValue> &A = (swapped ? old_seq : new_seq), &B = (swapped ? new_seq : old_seq);
	const size_t M = std::size(A), N = std::size(B), delta = N - M, offset = M + 1;

	struct P { long long x; long long y; long long k; };
	std::vector<P> path_coordinates;
	std::vector<long long> path(M + N + 3, -1);

	auto snake = [&](const long long& k, const long long& above, const long long& below) -> long long {
		const long long r = above > below ? path[(size_t)k - 1 + offset] : path[(size_t)k + 1 + offset];

		long long y = (above > below) ? above : below;
		long long x = y - k;
		while ((size_t)x < M && (size_t)y < N && (A[(size_t)x] == B[(size_t)y])) {
			++x; ++y;
		}

		path[(size_t)k + offset] = static_cast<long long>(path_coordinates.size());
		path_coordinates.push_back(P{ x, y, r });

		return y;
	};

	std::vector<long long> fp(M + N + 3, -1);
	for (long long p = 0; ; ++p) {
		for (long long k = -p; k <= static_cast<long long>(delta) - 1; ++k) {
			fp[size_t(k + offset)] = snake(k, fp[size_t(k - 1 + offset)] + 1, fp[size_t(k + 1 + offset)]);
		}
		for (long long k = static_cast<long long>(delta) + p; k >= static_cast<long long>(delta) + 1; --k) {
			fp[size_t(k + offset)] = snake(k, fp[size_t(k - 1 + offset)] + 1, fp[size_t(k + 1 + offset)]);
		}
		fp[delta + offset] = snake(static_cast<long long>(delta), fp[delta - 1 + offset] + 1, fp[delta + 1 + offset]);

		if (fp[delta + offset] == static_cast<long long>(N)) break;
	}

	std::vector<P> epc;
	for (long long r = path[delta + offset]; r != -1;) {
		const auto& p = path_coordinates[size_t(r)];
		epc.push_back(P{ p.x, p.y, 0 });
		r = p.k;
	}

	auto report_change = [&](long long x, long long y) {
		auto a = (x >= 0) ? &A[size_t(x)] : nullptr;
		auto b = (y >= 0) ? &B[size_t(y)] : nullptr;
		if (swapped) func(b, a); else func(a, b);
	};

	std::deque<long long> xs, ys;
	auto report_changes = [&](bool all) {
		const auto xsize = xs.size(), ysize = ys.size();
		const auto min_size = (xsize < ysize) ? xsize : ysize;
		for (size_t i = 0; i < min_size; ++i) {
			report_change(xs[i], ys[i]);
		}
		if (all) {
			for (size_t i = min_size; i < xsize; ++i) {
				report_change(xs[i], -1);
			}
			xs.clear();
			for (size_t i = min_size; i < ysize; ++i) {
				report_change(-1, ys[i]);
			}
			ys.clear();
		} else {
			xs.erase(xs.begin(), xs.begin() + min_size);
			ys.erase(ys.begin(), ys.begin() + min_size);
		}
	};

	auto record_change = [&](long long x, long long y) {
		if ((x >= 0) && (y >=0)) {
			report_changes(true);
			report_change(x, y);
		} else if (x >= 0) {
			xs.push_back(x);
			report_changes(false);
		} else if (y >= 0) {
			ys.push_back(y);
			report_changes(false);
		} else {
			/// should not reach here
		}
	};

	long long x = 0, y = 0; // coordinates
	for (size_t i = epc.size(); i != 0; --i) {
		const auto& p = epc[i - 1];
		while ((x < p.x) || (y < p.y)) {
			if ((p.y - p.x) >(y - x)) {
				record_change(-1, y);
				++y;
			} else if ((p.y - p.x) < (y - x)) {
				record_change(x, -1);
				++x;
			} else {
				record_change(x, y);
				++x; ++y;
			}
		}
	}
	report_changes(true);
}

