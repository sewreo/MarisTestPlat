
#ifndef CORE_EXPORT_H
#define CORE_EXPORT_H

#ifdef AUTOMATIONCORE_EXPORTS
#define AUTOMATIONCORE_API __declspec(dllexport)
#else
#define AUTOMATIONCORE_API __declspec(dllimport)
#endif

#endif