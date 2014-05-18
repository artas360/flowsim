#ifndef __KEY_GENERATOR_HPP__
#define __KEY_GENERATOR_HPP__


template <typename key_t=size_t>
class Key_generator {
    public:
        Key_generator(key_t seed = key_t(1)) : counter_(seed) {}

        key_t next() {
            return ++counter_;
        }

        key_t operator()() {
            return next();
        }

        static key_t is_valid_key(key_t key) {
            return key != no_key;
        }

        static const key_t not_key() {
            return no_key;
        }

    private:
        key_t counter_;
        static const key_t no_key = 0;
};

#endif
