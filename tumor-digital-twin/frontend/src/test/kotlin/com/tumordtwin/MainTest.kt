package com.tumordtwin

import io.kotest.core.spec.style.StringSpec
import io.kotest.matchers.shouldBe

/**
 * Basic test to verify the test infrastructure is working.
 */
class MainTest : StringSpec({
    "project structure should be initialized" {
        // Basic sanity test
        val projectName = "Tumor Digital Twin"
        projectName.length shouldBe 19
    }
})
