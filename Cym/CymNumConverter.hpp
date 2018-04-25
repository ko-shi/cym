#ifndef CYM_NUM_CONVERTER_HPP
#define CYM_NUM_CONVERTER_HPP

#include<limits>
#include"CymTCPair.hpp"
#include"CymVector.hpp"
#include"CymStringOperation.hpp"
#include"CymDataTypes.hpp"

#ifdef  max

#undef max

#endif //  max



namespace cym {


	Pair<bool, StrView> toUint(const StrView &str,Uint &out) {

		using Limit = std::numeric_limits<Uint>;

		const auto conv = [](const Vector<std::int8_t> &buf) {
			Uint res = 0;
			for (auto i : buf) {
				res *= 10;
				res += Uint(i);
			}
			return res;
		};

		if (str.empty()) {
			return makePair(false, StrView(u""));
		}

		Vector<std::int8_t> buf(Limit::digits10);
		for (auto i : str) {
			if (u'0' <= i && i <= u'9') {
				buf.pushBack(std::int8_t(i - u'0'));
			}
			else {
				if (buf.isEmpty()) {
					return makePair(false, StrView(u""));
				}
				out = conv(buf);
				return makePair(true, str.substr(0,buf.size()));
			}
		}
		out = conv(buf);
		return makePair(true,str);
	}
	Pair<bool, StrView> toInt(const StrView &str,Int &out) {
		if (str.empty()) {
			return makePair(false, StrView(u""));
		}
		if (str[0] == '-') {
			Uint uout;
			const auto temp = toUint(str.substr(1),uout);
			if (!temp.first) {
				return makePair(false, StrView(u""));
			}
			out = -Int(uout);
			return makePair(true, rangeOf(str,temp.second));
		}
		Uint uout;
		const auto temp = toUint(str,uout);
		if (!temp.first) {
			return makePair(false, StrView(u""));
		}
		out = Int(uout);
		return makePair(true,temp.second);
	}
	Int toInt(const StrView &str) {
		Int i = 0;
		toInt(str, i);
		return i;
	}
	Pair<bool, StrView> toDouble(const StrView &str,double &out,Int *out_int = nullptr) {
		const auto getDecimalPart = [](Uint i) {
			double temp = 0.;
			while (i > 0u) {
				temp += i % 10;
				temp /= 10;
				i /= 10;
			}
			return temp;
		};

		const auto until_period = takeWhile(str, [](Char c) {return c != u'.'; });
		Int first_half_val;
		const auto first_half = toInt(until_period,first_half_val); 
		if (!first_half.first) {
			return makePair(false, StrView(u""));
		}
		double buf = 0.;
		buf += static_cast<double>(first_half_val);

		const auto remained = getRemainedStr(str, until_period);
		if (remained.empty()) {
			out = buf;
			if (out_int)*out_int = first_half_val;
			return makePair(false, first_half.second);
		}
		const auto after_period = remained.substr(1);
		Uint last_half_value;
		const auto last_half = toUint(after_period,last_half_value);
		if (!last_half.first) {
			return makePair(false, StrView(u""));
		}
		if(out_int)*out_int = first_half_val;
		out = buf + getDecimalPart(last_half_value);
		return makePair(true,rangeOf(str,last_half.second));
	}
	double toDouble(const StrView &str) {
		double d = 0.;
		toDouble(str, d);
		return d;
	}
	Pair<std::variant<double, Int>,StrView> getNumKind(const StrView &str,TokenKind &kind) {
		double result;
		Int result_if_int;
		const auto try_double = toDouble(str,result,&result_if_int);
		if (!try_double.first) {
			if (try_double.second.empty()) {
				kind = TokenKind::ERROR;
				return makePair(std::variant<double, Int>(0.),StrView(u""));
			}
			else {
				kind = TokenKind::NUMBER;
				return makePair(std::variant<double, Int>(result_if_int), try_double.second);
			}
		}
		kind = TokenKind::DECIMAL;
		return makePair(std::variant<double, Int>(result),try_double.second);
	}

}


#endif