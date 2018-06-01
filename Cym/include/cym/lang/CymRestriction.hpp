#ifndef CYM_RESTRICTION_HPP
#define CYM_RESTRICTION_HPP

#include<cym/CymBase.hpp>
#include<utility>

#ifdef CONST
#	undef CONST
#endif

namespace cym {


	enum class RestId {
		ANY,CONST,PARAM,FUNC,INSTANCE_OF
	};

	struct RestBase {
		virtual RestId id()const = 0;
		virtual bool compare(const RestBase *a)const {
			return a->id() == id();
		}
		virtual ~RestBase() {
			
		}
	};

	struct RestAny : RestBase {
		virtual RestId id()const override {
			return RestId::ANY;
		}
	};
	struct RestConst : RestBase {
		virtual RestId id()const override {
			return RestId::CONST;
		}
	};

	struct RestParam : RestBase {
		virtual RestId id()const override {
			return RestId::PARAM;
		}
	};

	struct RestFunc : RestBase {
		virtual RestId id()const override {
			return RestId::FUNC;
		}
	};
	struct RestInstanceOf : RestBase {
		Str name;
		virtual bool compare(const RestBase *a)const override{
			if (a->id() == RestId::FUNC) {
				return name == dynamic_cast<const RestInstanceOf*>(a)->name;
			}
			return false;
		}
		virtual RestId id()const override {
			return RestId::FUNC;
		}
	};


	struct Restriction {
		struct Equal {
			bool operator()(const std::unique_ptr<RestBase> &l, const std::unique_ptr<RestBase> &r) const{
				return l->compare(r.get());
			}
		};
		std::unordered_set<std::unique_ptr<RestBase>,std::hash<std::unique_ptr<RestBase>>,Equal> rest;

		bool operator==(const Restriction &r) {
			if (rest.size() != r.rest.size()) {
				return false;
			}
			for (const auto &i : rest) {
				if (r.rest.find(i) == r.rest.end()) {
					return false;
				}
			}
			return true;
		}
	};
}

#endif