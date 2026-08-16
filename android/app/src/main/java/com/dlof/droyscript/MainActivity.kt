package com.dlof.droyscript

import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.dlof.droyscript.engine.DroyEngine

class MainActivity : AppCompatActivity() {

    private lateinit var engine: DroyEngine
    private lateinit var editor: EditText

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        engine = DroyEngine()

        editor = findViewById(R.id.editor)
        val output = findViewById<TextView>(R.id.output)
        val runButton = findViewById<Button>(R.id.runButton)
        val loadSampleButton = findViewById<Button>(R.id.loadSampleButton)

        editor.setText(DroyEngine.SAMPLE_SCRIPT)

        runButton.setOnClickListener {
            val source = editor.text.toString()
            val result = engine.run(source)
            output.text = result
        }

        loadSampleButton.setOnClickListener {
            editor.setText(DroyEngine.SAMPLE_SCRIPT)
        }
    }
}
