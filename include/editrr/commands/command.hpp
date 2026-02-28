#pragma once
namespace editrr {

    struct EditorContext; // forward

    struct ICommand {
        virtual ~ICommand() = default;
        virtual void execute(EditorContext& ctx) = 0;
    };

} // namespace editrr