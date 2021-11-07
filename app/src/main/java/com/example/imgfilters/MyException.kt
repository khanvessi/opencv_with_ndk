package com.example.imgfilters

import java.lang.Exception

class MyException (
    /**
     * @param message the message to set
     */
    override var message: String
) : Exception() {
    /**
     * @return the message
     */

    companion object {
        /**
         *
         */
        private const val serialVersionUID = 1L
    }
}