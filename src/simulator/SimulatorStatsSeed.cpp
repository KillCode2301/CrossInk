#ifdef SIMULATOR

#include "SimulatorStatsSeed.h"

#include <Logging.h>

#include <cstdlib>

#include "activities/reader/GlobalReadingStats.h"
#include "activities/reader/ReadingStatsUtils.h"

namespace {

bool seedStatsEnabled() {
  const char* raw = std::getenv("CROSSINK_SIMULATOR_SEED_STATS");
  return raw != nullptr && raw[0] != '\0' && raw[0] != '0';
}

void markDayOffset(GlobalReadingStats& stats, const ReadingStatsDate& anchorDate, const int dayOffset) {
  ReadingStatsDate date = anchorDate;
  addDaysToReadingStatsDate(date, dayOffset);
  if (!date.isValid()) {
    return;
  }

  ReadingStatsDateTime spanStart{};
  spanStart.date = date;
  spanStart.hour = 20;
  spanStart.minute = 0;
  recordReadingSpanIntoHistory(stats.readingHistoryAnchorDay, stats.readingHistoryBits, spanStart, 15 * 60);
}

}  // namespace

void seedSimulatorReadingStatsIfRequested() {
  if (!seedStatsEnabled()) {
    return;
  }
  if (!shouldShowRtcBasedReadingStats()) {
    LOG_ERR("SIM", "CROSSINK_SIMULATOR_SEED_STATS requires CROSSINK_SIMULATOR_FAKE_RTC");
    return;
  }

  ReadingStatsDateTime today{};
  if (!getCurrentLocalReadingStatsDateTime(today)) {
    LOG_ERR("SIM", "Failed to read fake RTC date for stats seed");
    return;
  }

  GlobalReadingStats stats;
  stats.totalSessions = 38;
  stats.totalReadingSeconds = 25u * 3600u;
  stats.totalPagesTurned = 940;
  stats.completedBooks = 4;
  stats.timeOfDaySeconds[static_cast<size_t>(ReadingTimeBucket::Morning)] = 3u * 3600u;
  stats.timeOfDaySeconds[static_cast<size_t>(ReadingTimeBucket::Evening)] = 7u * 3600u;
  stats.dayOfWeekSeconds[1] = 4u * 3600u;
  stats.dayOfWeekSeconds[4] = 6u * 3600u;

  // Today plus a spread of days in this month and last month for the monthly row.
  markDayOffset(stats, today.date, 0);
  markDayOffset(stats, today.date, -1);
  markDayOffset(stats, today.date, -3);
  markDayOffset(stats, today.date, -5);
  markDayOffset(stats, today.date, -8);
  markDayOffset(stats, today.date, -12);
  markDayOffset(stats, today.date, -20);
  markDayOffset(stats, today.date, -35);
  markDayOffset(stats, today.date, -40);
  markDayOffset(stats, today.date, -42);

  stats.longestReadingStreak = computeReadingHistoryLongestStreak(stats.readingHistoryAnchorDay, stats.readingHistoryBits);
  stats.save();

  const uint16_t thisMonthDays = stats.daysReadInMonth(today.date.year, today.date.month);
  uint16_t lastMonthYear = 0;
  uint8_t lastMonthMonth = 0;
  if (today.date.month <= 1) {
    lastMonthYear = static_cast<uint16_t>(today.date.year - 1);
    lastMonthMonth = 12;
  } else {
    lastMonthYear = today.date.year;
    lastMonthMonth = static_cast<uint8_t>(today.date.month - 1);
  }
  const uint16_t lastMonthDays = stats.daysReadInMonth(lastMonthYear, lastMonthMonth);

  LOG_INF("SIM", "Seeded global reading stats (this month: %u days, last month: %u days, streak: %u)",
          static_cast<unsigned>(thisMonthDays), static_cast<unsigned>(lastMonthDays),
          static_cast<unsigned>(stats.currentReadingStreak(&today.date)));
}

#endif
