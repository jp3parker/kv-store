
#pragma once

enum class RecoveryResult {
    Success,
    Truncated,
    Corrupt,
    ReadError,
    InvalidOperation,
    ChecksumMismatch
};
