#pragma once

namespace util {
	template<size_t... Is>
	struct index_sequence {};

	namespace details {


		template<size_t S>
		struct index_sequence_generator {
			template<class Seq>
			struct append_sequence;

			template<size_t... Is>
			struct append_sequence<index_sequence<Is...>>
			{
				using type = index_sequence<Is..., S - 1>;
			};

			using type = typename append_sequence<typename index_sequence_generator<S - 1>::type>::type;
		};

		template<>
		struct index_sequence_generator<0> {
			using type = index_sequence<>;
		};
	}

	template<size_t S>
	typename details::index_sequence_generator<S>::type make_index_sequence()
	{
		return typename details::index_sequence_generator<S>::type();
	}
}
