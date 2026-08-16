package com.dlof.droyscript.engine

/**
 * Thin Kotlin wrapper around the native droy script x engine (C++17,
 * lexer/parser/interpreter compiled into libdroyengine.so). Mirrors the
 * pattern used by the RinLang JNI shell.
 */
class DroyEngine {

    companion object {
        init {
            System.loadLibrary("droyengine")
        }

        /** Reference program shown to first-time users. */
        const val SAMPLE_SCRIPT = """@see.droy=start

@group Users
    ${'$'}row id=1 name="Ali"  number=100
    ${'$'}row id=2 name="Omar" number=200
    ${'$'}row id=3 name="Sara" number=100
@end

@collection ActiveUsers <= Users.equals(number=100)
@end

@bridge Friendship
    edge id=1 => id=2
    edge id=2 => id=3
@end

@map Settings
    "theme"   => "dark"
    "version" => 1
@end

~print(Users)
~print(ActiveUsers)
~print(Friendship)
~print(Settings)
~len(Users)
~sum(Users, number)

@see.droy=end
"""
    }

    /** Runs a droy script program and returns its combined ~print/~len/~sum output. */
    external fun nativeRun(source: String): String

    /** Kotlin-friendly entry point; never throws — errors come back as "ERROR: ..." text. */
    fun run(source: String): String = nativeRun(source)
}
