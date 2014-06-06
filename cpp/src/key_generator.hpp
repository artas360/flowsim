#ifndef __KEY_GENERATOR_HPP__
#define __KEY_GENERATOR_HPP__


/**
 * \class Key_generator
 * \brief Class used to generate unique keys.
 */
template <typename key_t=size_t>
class Key_generator {
    public:
        Key_generator(key_t seed = key_t(1)) : counter_(seed) {}

        /**
         * \brief Returns the next key.
         * \return The next unique key.
         * \warning The first call will return seed + 1, not seed.
         */
        key_t next() {
            return ++counter_;
        }

        /**
         * \brief Forwards call to next().
         * \return The next unique key.
         */
        key_t operator()() {
            return next();
        }

        /**
         * \brief Checks whether a key value is valid or not.
         * \param key Key to be tested.
         * \return True if the key is valid, else false.
         */
        static key_t is_valid_key(key_t key) {
            return key != no_key;
        }

        /**
         * \brief Returns a non valid key.
         * \return Non valid key.
         *
         * is_valid_key(not_key()) always returns false.
         */
        static const key_t not_key() {
            return no_key;
        }

    private:
        key_t counter_;
        static const key_t no_key = 0;
};

#endif
