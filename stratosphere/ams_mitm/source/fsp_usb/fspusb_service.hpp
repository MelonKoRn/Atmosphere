
#pragma once
#include "impl/fspusb_usb_manager.hpp"
#include "fspusb_filesystem.hpp"
#include <stratosphere/fssrv/fssrv_interface_adapters.hpp>

using IFileSystemInterface = ams::fssrv::impl::FileSystemInterfaceAdapter;

namespace ams::mitm::fspusb {

    class Service : public sf::IServiceObject {

        private:
            enum class CommandId {
                ListMountedDrives = 0,
                GetDriveFileSystemType = 1,
                GetDriveLabel = 2,
                SetDriveLabel = 3,
                OpenDriveFileSystem = 4,
            };

        public:
            void ListMountedDrives(const sf::OutArray<s32> &out_interface_ids, sf::Out<s32> out_count) {
                impl::DoUpdateDrives();
                size_t drive_count = impl::GetAcquiredDriveCount();
                size_t buf_drive_count = std::min(drive_count, out_interface_ids.GetSize());

                for(u32 i = 0; i < buf_drive_count; i++) {
                    out_interface_ids[i] = impl::GetDriveInterfaceId(i);
                }

                out_count.SetValue(static_cast<s32>(buf_drive_count));
            }

            Result GetDriveFileSystemType(s32 drive_interface_id, sf::Out<u8> out_fs_type) {
                impl::DoUpdateDrives();
                R_UNLESS(impl::IsDriveInterfaceIdValid(drive_interface_id), ResultInvalidDriveIndex());

                impl::DoWithDriveFATFS(drive_interface_id, [&](FATFS *fs) {
                    out_fs_type.SetValue(fs->fs_type);
                });

                return ResultSuccess();
            }

            Result GetDriveLabel(s32 drive_interface_id, sf::OutBuffer &out_label_str) {
                impl::DoUpdateDrives();
                R_UNLESS(impl::IsDriveInterfaceIdValid(drive_interface_id), ResultInvalidDriveIndex());

                auto drive_mounted_idx = impl::GetDriveMountedIndex(drive_interface_id);
                char mountname[0x10] = {0};
                impl::FormatDriveMountName(mountname, drive_mounted_idx);

                auto ffrc = f_getlabel(mountname, reinterpret_cast<char*>(out_label_str.GetPointer()), nullptr);
                return result::CreateFromFRESULT(ffrc);
            }

            Result SetDriveLabel(s32 drive_interface_id, sf::InBuffer &label_str) {
                impl::DoUpdateDrives();
                R_UNLESS(impl::IsDriveInterfaceIdValid(drive_interface_id), ResultInvalidDriveIndex());

                auto drive_mounted_idx = impl::GetDriveMountedIndex(drive_interface_id);
                char mountname[0x10] = {0};
                impl::FormatDriveMountName(mountname, drive_mounted_idx);

                char label[0x10] = {0};
                /* Check that no more than 11 characters are copied */
                snprintf(label, 11, "%s", reinterpret_cast<const char*>(label_str.GetPointer()));

                char newname[0x100] = {0};
                sprintf(newname, "%s%s", mountname, label);

                auto ffrc = f_setlabel(newname);
                return result::CreateFromFRESULT(ffrc);
            }

            Result OpenDriveFileSystem(s32 drive_interface_id, sf::Out<std::shared_ptr<IFileSystemInterface>> out_fs) {
                impl::DoUpdateDrives();
                R_UNLESS(impl::IsDriveInterfaceIdValid(drive_interface_id), ResultInvalidDriveIndex());

                std::shared_ptr<fs::fsa::IFileSystem> drv_fs = std::make_shared<DriveFileSystem>(drive_interface_id);
                out_fs.SetValue(std::make_shared<IFileSystemInterface>(std::move(drv_fs), false));

                return ResultSuccess();
            }

            DEFINE_SERVICE_DISPATCH_TABLE {
                MAKE_SERVICE_COMMAND_META(ListMountedDrives),
                MAKE_SERVICE_COMMAND_META(GetDriveFileSystemType),
                MAKE_SERVICE_COMMAND_META(GetDriveLabel),
                MAKE_SERVICE_COMMAND_META(SetDriveLabel),
                MAKE_SERVICE_COMMAND_META(OpenDriveFileSystem),
            };
    };

}