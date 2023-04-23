#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> Log::core_logger_;
std::shared_ptr<spdlog::logger> Log::client_logger_;

void Log::Init()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");
	core_logger_ = spdlog::stdout_color_mt("CORE");
	core_logger_->set_level(spdlog::level::trace);

	client_logger_ = spdlog::stdout_color_mt("APP");
	client_logger_->set_level(spdlog::level::trace);

}
