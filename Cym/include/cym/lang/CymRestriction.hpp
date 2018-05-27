#ifndef CYM_RESTRICTION_HPP
#define CYM_RESTRICTION_HPP

#include<cym/lang/CymIntermediateCodeGenerator.hpp>

namespace cym {

	struct VarTag {
		bool operator==(VarTag)const { return true; }
	};
	struct ConstTag {
		bool operator==(ConstTag)const { return true; }
	};
	struct ConcreteClassTag {
		Tree *icode_identifier;
		bool operator==(ConcreteClassTag c)const { return icode_identifier == c.icode_identifier; }
	};

	// Restriction data
	using RData = std::variant<
		VarTag,
		ConstTag,
		ConcreteClassTag
	>;

	bool operator==(const RData &l, const RData &r) {
		if (l.index() != r.index()) {
			return false;
		}
		return std::visit([](const auto &l_, auto &r_) {return l_ == r_; }, l, r);
	}

	class Restriction {
	public:
	private:
		Vector<RData> restriction;
	public:
		Restriction() {

		}
		Restriction(const Restriction &r) : restriction(r.restriction) {

		}
		Restriction(Restriction &&r) : restriction(std::move(r.restriction)) {

		}
		Restriction(const Vector<RData> &r) : restriction(r) {

		}
		void add(const RData &r) {
			restriction.pushBack(r);
		}
		void add(const Restriction &r) {
			restriction.pushBack(r.restriction);
			std::sort(restriction.begin(), restriction.end());
			const auto itr = std::unique(restriction.begin(), restriction.end());
			restriction.reduceSize(restriction.end() - itr);
		}
		bool satisfy(const Restriction &r)const {
			// O(n)!!!!!!!
			for (auto i : r.restriction) {
				if (std::find(restriction.begin(), restriction.end(), i) == restriction.end()) {
					return false;
				}
			}
			true;
		}
		Size priority()const {
			return restriction.size();
		}
	};
}

#endif