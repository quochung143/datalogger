# Display Preferences - User Guide

Welcome to the DataLogger Display Preferences feature! This guide will help you customize how temperature, time, and date values are displayed throughout the dashboard.

---

## Table of Contents
1. [Overview](#overview)
2. [Accessing Settings](#accessing-settings)
3. [Temperature Unit](#temperature-unit)
4. [Time Format](#time-format)
5. [Date Format](#date-format)
6. [Saving Settings](#saving-settings)
7. [FAQ](#faq)

---

## Overview

The Display Preferences feature lets you customize **three key elements**:

- 🌡️ **Temperature Unit** - How temperature values are displayed
- 🕐 **Time Format** - 24-hour or 12-hour (AM/PM) time display
- 📅 **Date Format** - Different date arrangement styles

Your preferences are **automatically saved** and will persist even after closing and reopening your browser.

---

## Accessing Settings

### Step 1: Open the Dashboard
Navigate to your DataLogger Monitor dashboard (e.g., http://localhost:3000)

### Step 2: Click "Settings" in the Sidebar
- Located in the left navigation menu
- Icon: ⚙️ Settings

### Step 3: Scroll to "Display Preferences"
The section is organized by category. Scroll down to find the Display Preferences section with the eye icon 👁️

---

## Temperature Unit

### Option 1: Celsius (°C) - **Default**
- **Display:** 25.48°C
- **Best for:** Scientific applications, most of the world
- **Example values:**
  - Freezing point of water: 0°C
  - Room temperature: 22°C
  - Body temperature: 37°C

### Option 2: Fahrenheit (°F)
- **Display:** 77.86°F
- **Best for:** USA and specific regions
- **Example values:**
  - Freezing point of water: 32°F
  - Room temperature: 72°F
  - Body temperature: 98.6°F

### Option 3: Kelvin (K)
- **Display:** 298.63K
- **Best for:** Scientific and academic purposes
- **Example values:**
  - Freezing point of water: 273.15K
  - Room temperature: 295K
  - Body temperature: 310K

### Where Temperature Appears
After changing the temperature unit, it will display in your selected format in:
- 📊 **Dashboard** - Current reading card
- 📋 **Live Data table** - Real-time measurements
- 📁 **Data Management** - Historical data table
- 📈 **Chart labels** - Y-axis label
- 📥 **Exported CSV files** - Column headers and values

---

## Time Format

### Option 1: 24-hour - **Default**
- **Display:** 14:30:45
- **Best for:** International standard, military time
- **Examples:**
  - Midnight: 00:00:00
  - Noon: 12:00:00
  - 2:30 PM: 14:30:00
  - 8:45 PM: 20:45:00

### Option 2: 12-hour (AM/PM)
- **Display:** 02:30:45 PM
- **Best for:** USA and English-speaking countries
- **Examples:**
  - Midnight: 12:00:00 AM
  - Noon: 12:00:00 PM
  - 2:30 PM: 02:30:45 PM
  - 8:45 PM: 08:45:45 PM

### Where Time Appears
After changing the time format, it will display in your selected format in:
- 📊 **Dashboard** - "Updated at [TIME]"
- 📋 **Live Data table** - Time column
- 📁 **Data Management** - Date/Time column
- 📥 **Exported CSV files** - Time values

---

## Date Format

### Option 1: DD/MM/YYYY - **Default**
- **Display:** 26/10/2025
- **Best for:** Europe, Australia, most of the world
- **Breakdown:**
  - DD = Day (01-31)
  - MM = Month (01-12)
  - YYYY = Year (2025)

### Option 2: MM/DD/YYYY
- **Display:** 10/26/2025
- **Best for:** USA and English-speaking countries
- **Breakdown:**
  - MM = Month (01-12)
  - DD = Day (01-31)
  - YYYY = Year (2025)

### Option 3: YYYY-MM-DD
- **Display:** 2025-10-26
- **Best for:** ISO standard, databases, international
- **Breakdown:**
  - YYYY = Year (2025)
  - MM = Month (01-12)
  - DD = Day (01-31)

### Where Date Appears
After changing the date format, it will display in your selected format in:
- 📁 **Data Management** - Date/Time column
- 📥 **Exported CSV files** - Date values
- 📊 **Historical data queries** - Date displays

---

## Saving Settings

### Step 1: Make Your Selections
1. Open **Settings** → **Display Preferences**
2. Choose your preferred:
   - Temperature Unit (dropdown)
   - Time Format (dropdown)
   - Date Format (dropdown)

### Step 2: Click "Save Settings"
- Located at the bottom of the page
- Green button with checkmark ✓

### Step 3: Confirm Success
You should see a success message:
> "Settings saved successfully!"

### Step 4: Verify Changes
The dashboard should immediately reflect your preferences:
- Go to **Dashboard** to see current reading with new format
- Go to **Live Data** to see table with new time/temperature format
- Go to **Data Management** to see all formats applied

---

## Quick Reference Table

### Temperature Unit Comparison
| Celsius | Fahrenheit | Kelvin |
|---------|-----------|--------|
| 0°C | 32°F | 273.15K |
| 10°C | 50°F | 283.15K |
| 20°C | 68°F | 293.15K |
| 25°C | 77°F | 298.15K |
| 30°C | 86°F | 303.15K |
| 37°C | 98.6°F | 310.15K |

### Time Format Comparison
| 24-hour | 12-hour |
|---------|---------|
| 00:00:00 | 12:00:00 AM |
| 06:00:00 | 06:00:00 AM |
| 12:00:00 | 12:00:00 PM |
| 14:30:00 | 02:30:00 PM |
| 18:45:00 | 06:45:00 PM |
| 23:59:59 | 11:59:59 PM |

### Date Format Comparison
| DD/MM/YYYY | MM/DD/YYYY | YYYY-MM-DD |
|-----------|-----------|-----------|
| 26/10/2025 | 10/26/2025 | 2025-10-26 |
| 01/01/2025 | 01/01/2025 | 2025-01-01 |
| 25/12/2025 | 12/25/2025 | 2025-12-25 |

---

## FAQ

### Q: Will my preferences be saved if I close the browser?
**A:** Yes! Your preferences are saved in your browser's local storage. They will remain saved even after closing and reopening your browser (unless you clear browser data).

### Q: Can I sync preferences across devices?
**A:** Currently, preferences are saved per device/browser. Future updates may include cloud sync across devices.

### Q: Do the preferences affect exported CSV files?
**A:** Yes! Exported files will include your selected date format, time format, and temperature unit in both the headers and the data values.

### Q: What if I want to reset to defaults?
**A:** Scroll to the bottom of the Settings page and click "Restore Defaults". This will reset:
- Temperature Unit → Celsius
- Time Format → 24-hour
- Date Format → DD/MM/YYYY

### Q: Why is my temperature still showing in Celsius after changing to Fahrenheit?
**A:** Try these steps:
1. Refresh the page (F5 or Ctrl+R)
2. Go back to Settings → Display Preferences
3. Click "Save Settings" again
4. Refresh the dashboard

If this doesn't work, clear your browser's cache and try again.

### Q: Can I have different preferences for different pages?
**A:** No, your preferences apply globally across all pages in the dashboard. However, you can change them anytime and see the updates immediately across all pages.

### Q: How does temperature conversion work?
**A:** The dashboard stores all temperatures internally in Celsius. When you select a different unit, the values are automatically converted:
- **Celsius to Fahrenheit:** °F = (°C × 9/5) + 32
- **Celsius to Kelvin:** K = °C + 273.15

### Q: Are my preferences backed up to the server?
**A:** No, preferences are stored locally in your browser. They are not synchronized with the DataLogger server. If you reinstall your browser or clear browser data, you'll lose your settings.

### Q: Can I export my preferences?
**A:** Yes! Go to Settings and click "Export Settings". This will download a JSON file containing all your preferences that you can later import using "Import Settings".

### Q: What happens if I import settings that have different preferences?
**A:** The imported settings will overwrite your current preferences. The dashboard will automatically refresh with the new settings.

---

## Support

If you encounter any issues with Display Preferences:

1. **Check the Settings page** - Verify your selections are correctly saved
2. **Refresh the page** - Sometimes a page refresh helps apply changes
3. **Clear browser cache** - May resolve display issues
4. **Check console logs** - Look for any error messages (F12 → Console tab)
5. **Contact support** - Include screenshots and describe what's not working

---

## Technical Details

### Storage Method
Display preferences are stored using browser localStorage:
```
tempUnit:   localStorage.getItem("tempUnit")
timeFormat: localStorage.getItem("timeFormat")
dateFormat: localStorage.getItem("dateFormat")
```

### Default Values
If you've never saved preferences:
- Temperature Unit: Celsius (°C)
- Time Format: 24-hour
- Date Format: DD/MM/YYYY

### Browser Compatibility
✅ Works in all modern browsers:
- Chrome/Chromium
- Firefox
- Safari
- Edge

---

## Tips & Tricks

### 💡 Tip 1: Export Before Changing Preferences
If you plan to export a large dataset, do it before changing preferences. This ensures consistent formatting throughout your exported file.

### 💡 Tip 2: Note Your Current Preferences
Before changing preferences, take a screenshot of the Settings page. This makes it easy to revert if needed.

### 💡 Tip 3: Test Small Changes
Try changing one preference at a time and saving. This helps you understand which display areas are affected.

### 💡 Tip 4: Use Keyboard Shortcuts
- Alt + S (Windows) or Option + S (Mac) - Often selects the Save Settings button

---

*Last Updated: 2025-10-26*
*For technical questions, see DEVELOPER_REFERENCE.md*
