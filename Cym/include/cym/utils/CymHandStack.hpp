#ifndef CYM_HAND_STACK_HPP
#define CYM_HAND_STACK_HPP


#include<cym/utils/CymTCVector.hpp>

namespace cym {

	template<class T, std::enable_if_t<std::is_trivially_copyable_v<T>, std::nullptr_t> = nullptr>
	struct DataSet {
		Size size;
		T additional_data;
	};
	template<class AdT,std::enable_if_t<std::is_trivially_copyable_v<AdT>, std::nullptr_t> = nullptr>
	class HandStack {
	private:
		using DataSetType = DataSet<AdT>;


		TCVector<std::uint8_t> buffer_;
	public:
		HandStack(Size size = 16) : buffer_(size){
			
		}
		bool isEmpty()const {
			return buffer_.isEmpty();
		}
		template<class T,std::enable_if_t<std::is_trivially_copyable_v<T>,std::nullptr_t> = nullptr>
		void pushBack(T* begin, T* end,const AdT &additional_data) {
			const auto size = (end - begin) * sizeof(T);
			buffer_.reserve(buffer_.size() + size + sizeof(DataSetType));
			std::memcpy(buffer_.end(), begin, size);
			buffer_.addSize(size);
			DataSetType dataset{ size,additional_data };
			std::memcpy(buffer_.end(), &dataset, sizeof(dataset));
			buffer_.addSize(sizeof(dataset));
		}

		void pushBack(Int data, const AdT &additional_data) {
			DataSetType dataset{ 4,additional_data};
			buffer_.reserve(buffer_.size() + 4 + sizeof(DataSetType));
			*static_cast<Int*>(static_cast<void*>(buffer_.end())) = data;
			std::memcpy(buffer_.end() + 4, &dataset, sizeof(dataset));
			buffer_.addSize(4 + sizeof(dataset));
		}

		AdT getLastAdditionalData() const {
			const DataSetType *dataset = static_cast<const DataSetType*>(static_cast<const void*>(buffer_.end() - sizeof(DataSetType)));
			return dataset->additional_data;
		}
		Size getLastSize() const {
			const DataSetType *dataset = static_cast<const DataSetType*>(static_cast<const void*>(buffer_.end() - sizeof(DataSetType)));
			return dataset->size;
		}

		std::pair<void*, void*> backAsArray() {
			const DataSetType *dataset = static_cast<const DataSetType*>(static_cast<const void*>(buffer_.end() - sizeof(DataSetType)));
			return std::make_pair(
				buffer_.end() - sizeof(DataSetType) - dataset->size
				, buffer_.end() - sizeof(DataSetType)
			);
		}
		void* backAsArrayBegin() {
			const DataSetType *dataset = static_cast<const DataSetType*>(static_cast<const void*>(buffer_.end() - sizeof(DataSetType)));
			return buffer_.end() - sizeof(DataSetType) - dataset->size;
		}

		Int backAs32() const{
			return *static_cast<const Int*>(static_cast<const void*>(buffer_.end() - sizeof(DataSetType) - 4));
		}

		/* Don't access the pointer of the result of backAsArray() after do this. */
		void popBack() {
			const DataSetType *dataset = static_cast<const DataSetType*>(static_cast<const void*>(buffer_.end() - sizeof(DataSetType)));
			buffer_.reduceSize(dataset->size + sizeof(DataSetType));
		}

	};
}


#endif